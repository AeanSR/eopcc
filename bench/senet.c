//======================================================================
//
//        filename :senet.c
//        description : simulate the Squeeze-and-Excitation Networks by EOP.
//
//        created by wgspring at 2019.8.14
//        name: Wang~Guichun
//        e-mail: wgspring@mail.ustc.edu.cn
//
//======================================================================

const int N = 1;
const int DEBUG = 1;

def log(x)
{
    //if (DEBUG)
        print x;
}

def squeeze_excitation_layer(x_ex)
{
    const int N = sizeof(x_ex) / sizeof(x_ex[0]);
    const int H = sizeof(x_ex[0]) / sizeof(x_ex[0][0]);
    const int W = sizeof(x_ex[0][0]) / sizeof(x_ex[0][0][0]);
    const int C = sizeof(x_ex[0][0][0]) / sizeof(x_ex[0][0][0][0]);
    int i;
    int j;
    int k;
    int l;

    // squeeze = Global_Average_Pooling(input_x)
    log("       Global_Average_Pooling");
    extern vector[N][1][1][C] squeezed_ex;
    EOPPooling(squeezed_ex, x_ex, H, W, 1, 1, 0, 0);

    // excitation = Fully_connected(squeeze, units = out_dim / ratio, layer_name = layer_name + '_fully_connected1')
    log("       flattend");
    extern vector[N][C] flattend_ex;
    {
        vector[N][C] flattend;
        vector[N][1][1][C] t;
        t = squeezed_ex;
        flattend = (vector[N][C])t;
        flattend_ex = flattend;
    }
    log("       Fully_connected");
    extern vector[N][C / 4] excitation_ex;
    extern vector[C / 4][C] weight;
    extern vector[C / 4] bias;
    EOPFullyConnected(excitation_ex, weight, flattend_ex, bias);

    // excitation = Fully_connected(excitation, units = out_dim, layer_name = layer_name + '_fully_connected2')
    log("       Fully_connected");
    extern vector[N][C] excitation2_ex;
    extern vector[C / 4][C] weight;
    extern vector[C] bias;
    EOPFullyConnected(excitation2_ex, weight, excitation_ex, bias);

    // scale = input_x * excitation
    // 点乘 [H,W,C]*[C]
    log("       scale = input_x * excitation");
    {
        vector[1][H][W][1] t_x;
        vector[C] t_excitation2;
        for (i = 0; i < N; i++)
        {
            t_excitation2 = (extern vector[C])excitation2_ex[i];
            for (j = 0; j < C; j++)
            {
                strideio t_x, (extern vector[0])x_ex[0][0][0][j], 1*2, C*2, H*W;
                // TODO 下面一句话执行时有点问题  undetermined data referenced as address!
                t_x *= t_excitation2[j];
                strideio(extern vector[0]) x_ex[0][0][0][j], t_x, 1*2, C*2, H*W;
            }
        }
    }
}

def residual_layer_block(x_ex, out_dim, r_x_ex)
{
    int i;
    int j;
    const int N = sizeof(x_ex) / sizeof(x_ex[0]);
    const int H = sizeof(x_ex[0]) / sizeof(x_ex[0][0]);
    const int W = sizeof(x_ex[0][0]) / sizeof(x_ex[0][0][0]);
    const int C = sizeof(x_ex[0][0][0]) / sizeof(x_ex[0][0][0][0]);

    const int stride = (2 * C != out_dim) ? 1 : 2;

    // splits = self.transform_layer(input_x, stride = stride, scope = layer_name + '_splitN_' + str(i))
    log("       transform_layer");
    extern vector[N][H / stride][W / stride][64] x_split_ex[8];
    for (i = 0; i < 8; i++) // transform_layer内要求分成8个split
    {
        extern vector[N][H][W][64] t;
        extern vector[64][1][1][C] weight;
        extern vector[64] bias;
        EOPConvolution(t, weight, x_ex, bias, 1, 1, 0, 0);

        extern vector[N][H / stride][W / stride][64] x_split;
        extern vector[64][3][3][64] weight;
        extern vector[64] bias;
        EOPConvolution(x_split, weight, t, bias, stride, stride, 1, 1);

        vector[1][H / stride][W / stride][64] t;
        for (j = 0; j < N; j++)
        {
            t = (extern vector[1][H / stride][W / stride][64]) x_split[j];
            (extern vector[1][H / stride][W / stride][64]) x_split_ex[i][j] = t;
        }
    }

    // Concatenation(layers_split)
    log("       Concatenation");
    extern vector[N][H / stride][W / stride][64 * 8] x2_ex;
    {
        vector[1][H / stride][W / stride][64] t;
        for (i = 0; i < N; i++)     //SPM分N片
            for (j = 0; j < 8; j++) //split的8片
            {
                t = (extern vector[1][H / stride][W / stride][64]) x_split_ex[j][i];
                (extern vector[1][H / stride][W / stride][64]) x2_ex[i][0][0][64 * j] = t;
            }
    }

    // // x = self.transition_layer(x, out_dim=out_dim, scope='trans_layer_'+layer_num+'_'+str(i))
    log("       transition_layer");
    // extern vector[N][H / stride][W / stride][out_dim] r_x_ex;
    extern vector[out_dim][1][1][64 * 8] weight;
    extern vector[out_dim] bias;
    EOPConvolution(r_x_ex, weight, x2_ex, bias, 1, 1, 0, 0);

    // // x = self.squeeze_excitation_layer(x, out_dim=out_dim, ratio=reduction_ratio, layer_name='squeeze_layer_'+layer_num+'_'+str(i))
    log("       squeeze_excitation_layer");
    squeeze_excitation_layer(r_x_ex);

    log("       pad_input_x");
    extern vector[N][H / stride][W / stride][out_dim] x_padded_ex;
    if (2 == stride)
    {
        // pad_input_x = Average_pooling(input_x)
        log("           Average_pooling");
        extern vector[N][H / 2][W / 2][C] x2_ex;
        EOPPooling(x2_ex, x_ex, 2, 2, 2, 2, 0, 0);

        // pad_input_x = tf.pad(pad_input_x, [[0, 0], [0, 0], [0, 0], [channel, channel]])
        {
            vector[H / 2][W / 2][out_dim] t_x_padded;
            vector[H / 2][W / 2][C] t_x2;
            for (i = 0; i < N; i++)
            {
                t_x_padded *= 0;
                (extern vector[H / 2][W / 2][out_dim]) x_padded_ex[i] = t_x_padded;
                t_x2 = (extern vector[H / 2][W / 2][C]) x2_ex[i];
                strideio (extern vector[0])x_padded_ex[i][0][0][C/2], t_x2, C*2, out_dim*2, 1;
            }
        }
    }
    else
    {
        // pad_input_x = input_x
        vector[W][H][C] t;
        for (i = 0; i < N; i++)
        {
            t = (extern vector[W][H][C])x_ex[i];
            (extern vector[W][H][C]) x_padded_ex[i] = t;
        }
    }

    // input_x = Relu(x + pad_input_x)
    log("       Relu(x + pad_input_x)");
    {
        vector[H][W][C] t_r_x;
        vector[H][W][C] t_x_padded;
        for (i = 0; i < N; i++)
        {
            t_r_x = (extern vector[H][W][C])r_x_ex[i];
            t_x_padded = (extern vector[H][W][C])x_padded_ex[i];

            t_r_x = t_r_x + t_x_padded;
            act t_r_x, t_r_x;

            (extern vector[H][W][C]) r_x_ex[i] = t_r_x;
        }
    }
}

def residual_layer(x_ex, out_dim, r_x_ex)
{
    const int N = sizeof(x_ex) / sizeof(x_ex[0]);
    const int H = sizeof(x_ex[0]) / sizeof(x_ex[0][0]);
    const int W = sizeof(x_ex[0][0]) / sizeof(x_ex[0][0][0]);
    const int C = sizeof(x_ex[0][0][0]) / sizeof(x_ex[0][0][0][0]);

    const int stride = (2 * C != out_dim) ? 1 : 2;

    extern vector[N][H / stride][W / stride][out_dim] x2_ex;
    extern vector[N][H / stride][W / stride][out_dim] x3_ex;
    log("   block1");
    residual_layer_block(x_ex, out_dim, x2_ex);
    log("   block2");
    residual_layer_block(x2_ex, out_dim, x3_ex);
    log("   block3");
    residual_layer_block(x3_ex, out_dim, r_x_ex);
}

// PROGRAME BEGAIN
log("PROGRAME BEGAIN...");
// input_x = self.first_layer(input_x, scope='first_layer')
log("first_layer...");
extern vector[N][32][32][3] X_ex;
extern vector[N][32][32][64] first_layer_ex; // 第一层（卷积）
extern vector[64][3][3][3] weight;
extern vector[64] bias;
EOPConvolution(first_layer_ex, weight, X_ex, bias, 1, 1, 1, 1);

// x = self.residual_layer(input_x, out_dim = 64, layer_num = '1')
log("residual_layer1...");
extern vector[N][32][32][64] res1_ex;
residual_layer(first_layer_ex, 64, res1_ex);

// x = self.residual_layer(x, out_dim = 128, layer_num = '2')
log("residual_layer2...");
extern vector[N][16][16][128] res2_ex;
residual_layer(res1_ex, 128, res2_ex);

// x = self.residual_layer(x, out_dim = 256, layer_num = '3')
log("residual_layer3...");
extern vector[N][8][8][256] res3_ex;
residual_layer(res2_ex, 256, res3_ex);

// global average pooling
log("global_pool...");
extern vector[N][1][1][256] global_pool_ex;
EOPPooling(global_pool_ex, res3_ex, 8, 8, 1, 1, 0, 0);

// flatten and FC
log("flatten...");
extern vector[N][256] flattened_ex;
{
    vector[N][256] flattened;
    vector[N][1][1][256] t;
    t = global_pool_ex;
    flattened = (vector[N][256])t;
    flattened_ex = flattened;
}

log("FC...");
extern vector[N][10] Y_ex;
extern vector[10][256] weight;
extern vector[10] bias;
EOPFullyConnected(Y_ex, weight, flattened_ex, bias);

log("PROGRAM FINISHED");
