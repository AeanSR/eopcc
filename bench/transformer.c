const int num_unit = 512;
const int num_hidden = 2048;
const int num_layer = 6;
const int num_head = 8;


const int bs = 2;
const int vector_length = 12;

def ElementWiseADD(A, B, C) {
    const int S = sizeof(A) / 2;
    const int MAX_S = 512 * 512;
    int i;
    const int time = S / MAX_S;
    const int last = S % MAX_S;
    vector[MAX_S] buf1;
    vector[MAX_S] buf2;
    for (i = 0; i < time; i++) {
    // (extern decltype(buf))((extern vector[0])A)[i * 262144]
        strideio buf1, (extern vector[0])A[i * MAX_S], MAX_S, 0, 1;
        strideio buf2, (extern vector[0])B[i * MAX_S], MAX_S, 0, 1;
        buf1 = buf1 + buf2;
        strideio (extern vector[0])C[i * MAX_S], buf1, MAX_S, 0, 1;
    }
    if (last > 0) {
        strideio buf1, (extern vector[0])A[time * MAX_S], last, 0, 1;
        strideio buf2, (extern vector[0])B[time * MAX_S], last, 0, 1;
        buf1 = buf1 + buf2;
        strideio (extern vector[0])C[time * MAX_S], buf1, last, 0, 1;
    }
}

def EOPCopy(dst, src) {
    const int S = sizeof(src);
    const int MAX_S = 512 * 512;
    int i;
    const int time = S / MAX_S;
    const int last = S % MAX_S;
    vector[MAX_S] buf1;
    for (i = 0; i < time; i++) {
    // (extern decltype(buf))((extern vector[0])A)[i * 262144]
        strideio buf1, (extern vector[0])src[i * MAX_S], MAX_S, 0, 1;
        strideio (extern vector[0])dst[i * MAX_S], buf1, MAX_S, 0, 1;
    }
    if (last > 0) {
        strideio buf1, (extern vector[0])src[time * MAX_S], last, 0, 1;
        strideio (extern vector[0])dst[time * MAX_S], buf1, last, 0, 1;
    }
}

def EOPSoftmax(In) {
    intern decltype(In) t;
    t = In;
    act t, t;
    t /= +t;
    In = t;
}

int rand = 10;

def get_rand(x, low, high) {
  rand = rand * 6364136223846793005ULL + 1442695040888963407ULL;
  x = ((float)((rand >> 1) & 0xFFFFFFFF) / 0x100000000) * (high - low) + low;
}

def sqrt(num, ans) {
    float tmp;
    float res;
    tmp = num;
    res = 1;
    res = (res + tmp / res) / 2.0;
    res = (res + tmp / res) / 2.0;
    res = (res + tmp / res) / 2.0;
    res = (res + tmp / res) / 2.0;
    res = (res + tmp / res) / 2.0;
    ans = res;
    // if (120. <= num && num <= 130.)
    //     ans = 11.31;
    // if (250. <= num && num <= 260.)
    //     ans = 16.0;
    // if (510. <= num && num <= 520.)
    //     ans = 22.6;
}

// assume smaller than buf
def EOPTranspose_1_2_vector(A, n, h, w, c, R) {
    // const int S = sizeof(A);
    const int buffer_size = n * h * c;
    vector[buffer_size] buf;
    int i;
    for(i = 0; i < w; i++) {
        strideio buf, (extern vector[0])A[0][0][i][0], c, w * c, n * h; // size stirde count
        strideio (extern vector[0])R[0][i][0][0], buf, h * c, h * w * c, n;
    }
}

def attention(Q, K, V, ANS) {
    vector[bs * num_head][vector_length][vector_length] score;
    vector[bs * num_head][vector_length][num_unit / num_head] buf1;
    vector[bs * num_head][vector_length][num_unit / num_head] buf2;

    strideio (vector[0])buf1, (extern vector[0])Q[0][0][0][0], bs*vector_length*num_unit, 0, 1;
    strideio (vector[0])buf2, (extern vector[0])K[0][0][0][0], bs*vector_length*num_unit, 0, 1;

    // score = batch_dot(Q, K_T)
    // mm score, buf1, buf2, 1;
    // res, weight, intput
    mm score, buf2, buf1;

    strideio (vector[0])buf1, (extern vector[0])V[0][0][0][0], bs*vector_length*num_unit, 0, 1;
    // score *= sqrt(d_k)
    int d;
    int scale;
    get_rand(d, 10, 20);
    sqrt(d, scale);
    score *= scale;

    EOPSoftmax(score);

    // ANS = batch_dot(score, V)
    mm buf2, buf1, score, 1;

    strideio (extern vector[0])ANS[0][0][0][0], (vector[0])buf2, bs*vector_length*num_unit, 0, 1;
}

def EOPLayerNorm(ToNorm, Normed) {
    float sum;
    float var;
    float gamma;
    float beta;
    int i;
    vector[vector_length * num_unit] window;
    vector[vector_length * num_unit] buf1;
    vector[vector_length * num_unit] buf2;
    for (i = 0; i < bs; i++) {
        strideio (vector[0])window, (extern vector[0])ToNorm[i * vector_length][0], vector_length * num_unit, 0, 1;
        sum = +window;
        sum = sum / (vector_length * num_unit);

        // buf1 : mean
        buf1 *= sum;

        // cal var
        buf2 = window - buf1;
        buf1 = buf2 * buf2;
        sqrt(+buf1, var);

        buf2 = buf2 * (gamma / var) + beta;
        strideio (extern vector[0])Normed[i][0][0], (vector[0])buf2, vector_length * num_unit, 0, 1;
    }
}

def MultiHeadAttention(Q, K, V, Output) {
    // MHA1  same input
    extern vector[num_unit][num_unit] MHA_Q_weight;
    extern vector[num_unit] MHA_Q_bias;
    extern vector[bs][vector_length][num_head][num_unit / num_head] MHA_Q;
    extern vector[bs][num_head][vector_length][num_unit / num_head] MHA_Q_T;

    extern vector[num_unit][num_unit] MHA_K_weight;
    extern vector[num_unit] MHA_K_bias;
    extern vector[bs][vector_length][num_head][num_unit / num_head] MHA_K;
    extern vector[bs][num_head][vector_length][num_unit / num_head] MHA_K_T;

    extern vector[num_unit][num_unit] MHA_V_weight;
    extern vector[num_unit] MHA_V_bias;
    extern vector[bs][vector_length][num_head][num_unit / num_head] MHA_V;
    extern vector[bs][num_head][vector_length][num_unit / num_head] MHA_V_T;

    extern vector[bs][num_head][vector_length][num_unit / num_head] ANS;
    extern vector[bs][vector_length][num_head][num_unit / num_head] ANS_T;

    EOPFullyConnected((extern vector[bs * vector_length][num_unit])MHA_Q,
                      MHA_Q_weight,
                      (extern vector[bs * vector_length][num_unit])Q,
                      MHA_Q_bias);
    EOPTranspose_1_2_vector(MHA_Q, bs, vector_length, num_head, num_unit / num_head, MHA_Q_T);
    EOPFullyConnected((extern vector[bs * vector_length][num_unit])MHA_K,
                      MHA_K_weight,
                      (extern vector[bs * vector_length][num_unit])K,
                      MHA_K_bias);
    EOPTranspose_1_2_vector(MHA_K, bs, vector_length, num_head, num_unit / num_head, MHA_K_T);
    EOPFullyConnected((extern vector[bs * vector_length][num_unit])MHA_V,
                      MHA_V_weight,
                      (extern vector[bs * vector_length][num_unit])V,
                      MHA_V_bias);
    EOPTranspose_1_2_vector(MHA_V, bs, vector_length, num_head, num_unit / num_head, MHA_V_T);

    attention(MHA_Q_T, MHA_K_T, MHA_V_T, ANS);
    EOPTranspose_1_2_vector(ANS, bs, num_head, vector_length, num_unit / num_head, ANS_T);

    extern vector[num_unit][num_unit] MHA_out_weight;
    extern vector[num_unit] MHA_out_bias;
    extern vector[bs * vector_length][num_unit] MHA_out;

    EOPFullyConnected(MHA_out,
                      MHA_out_weight,
                      (extern vector[bs * vector_length][num_unit])ANS,
                      MHA_out_bias);

    ElementWiseADD(Input, MHA_out, MHA_out);
    EOPLayerNorm(MHA_out, Output);

}

def encoder_layer(Input, Output) {
    extern vector[bs * vector_length][num_unit] encoder_MHA_out;
    MultiHeadAttention(Input, Input, Input, (extern vector[bs][vector_length][num_unit])encoder_MHA_out);

    // FFN
    extern vector[num_unit][num_hidden] encoder_FFN1_weight;
    extern vector[num_hidden] encoder_FFN1_bias;
    extern vector[bs * vector_length][num_hidden] encoder_FFN1;

    extern vector[num_hidden][num_unit] encoder_FFN2_weight;
    extern vector[num_unit] encoder_FFN2_bias;
    extern vector[bs * vector_length][num_unit] encoder_FFN2;

    EOPFullyConnected(encoder_FFN1, encoder_FFN1_weight, encoder_MHA_out, encoder_FFN1_bias);
    EOPFullyConnected(encoder_FFN2, encoder_FFN2_weight, encoder_FFN1, encoder_FFN2_bias);
    ElementWiseADD(encoder_MHA_out, encoder_FFN2, encoder_FFN2);
    EOPLayerNorm(encoder_FFN2, Output);
}


def decoder_layer(Input, encoder_output, Output) {
    extern vector[bs * vector_length][num_unit] decoder_MHA1_out;
    extern vector[bs * vector_length][num_unit] decoder_MHA2_out;

    MultiHeadAttention(Input, Input, Input, (extern vector[bs][vector_length][num_unit])decoder_MHA1_out);
    MultiHeadAttention(decoder_MHA1_out, encoder_output, encoder_output, (extern vector[bs][vector_length][num_unit])decoder_MHA2_out);

    // FFN
    extern vector[num_unit][num_hidden] decoder_FFN1_weight;
    extern vector[num_hidden] decoder_FFN1_bias;
    extern vector[bs * vector_length][num_hidden] decoder_FFN1;

    extern vector[num_hidden][num_unit] decoder_FFN2_weight;
    extern vector[num_unit] decoder_FFN2_bias;
    extern vector[bs * vector_length][num_unit] decoder_FFN2;

    EOPFullyConnected(decoder_FFN1, decoder_FFN1_weight, decoder_MHA2_out, decoder_FFN1_bias);
    EOPFullyConnected(decoder_FFN2, decoder_FFN2_weight, decoder_FFN1, decoder_FFN2_bias);
    ElementWiseADD(decoder_MHA2_out, decoder_FFN2, decoder_FFN2);
    EOPLayerNorm(decoder_FFN2, Output);
}


extern vector[bs][vector_length][num_unit] Input;
extern vector[bs][vector_length][num_unit] Output;
encoder_layer(Input, Input);
encoder_layer(Input, Input);
encoder_layer(Input, Input);
encoder_layer(Input, Input);
encoder_layer(Input, Input);
encoder_layer(Input, Input);
decoder_layer(Input, Input, Output);
decoder_layer(Input, Input, Output);
decoder_layer(Input, Input, Output);
decoder_layer(Input, Input, Output);
decoder_layer(Input, Input, Output);
decoder_layer(Output, Input, Output);
