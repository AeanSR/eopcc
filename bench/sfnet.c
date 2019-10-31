//======================================================================
//
//        filename :sfnet.c
//        description : simulate the ShuffleNet v2 0.5X by EOP.
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

def basic_unit_with_downsampling(x_ex, out_channels, r_x_ex, r_y_ex)
{
    // x[N,H,W,C]==>r_x[N,H/2,W/2,out_channels]
    // x[N,H,W,C]==>r_y[N,H/2,W/2,out_channels]
    const int N = sizeof(x_ex) / sizeof(x_ex[0]);
    const int H = sizeof(x_ex[0]) / sizeof(x_ex[0][0]);
    const int W = sizeof(x_ex[0][0]) / sizeof(x_ex[0][0][0]);
    const int C = sizeof(x_ex[0][0][0]) / sizeof(x_ex[0][0][0][0]);
    int i;

    // conv1x1_before
    log("       conv1x1_before");
    extern vector[N][H][W][C] y1_ex;
    extern vector[C][1][1][C] weight;
    extern vector[C] bias;
    EOPConvolution(y1_ex, weight, x_ex, bias, 1, 1, 0, 0);

    // depthwise  depthwise_conv(y, kernel=3, stride=2, activation_fn=None, scope='depthwise')
    log("       depthwise");
    extern vector[N][H / 2][W / 2][C] y2_ex;
    extern vector[C][3][3][C] weight;
    extern vector[C] bias;
    EOPDepthwiseConv(y2_ex, weight, y1_ex, bias, 2, 2, 1, 1);

    // conv1x1_after
    // vector[N][H / 2][W / 2][out_channels] r_y;
    log("       conv1x1_after");
    extern vector[out_channels][1][1][C] weight;
    extern vector[out_channels] bias;
    EOPConvolution(r_y_ex, weight, y2_ex, bias, 1, 1, 0, 0);

    // second_branch

    // depthwise
    log("       depthwise");
    extern vector[N][H / 2][W / 2][C] x1_ex;
    extern vector[C][3][3][C] weight;
    extern vector[C] bias;
    EOPDepthwiseConv(x1_ex, weight, x_ex, bias, 2, 2, 1, 1);

    // conv1x1_after
    // vector[N][H / 2][W / 2][out_channels] r_x;
    log("       conv1x1_after");
    extern vector[out_channels][1][1][C] weight;
    extern vector[out_channels] bias;
    EOPConvolution(r_x_ex, weight, x1_ex, bias, 1, 1, 0, 0);
}

def concat_shuffle_split(x_ex, y_ex)
{
    // x[N,H,W,C]==>r_x[N,H,W,C]
    // y[N,H,W,C]==>r_y[N,H,W,C]
    const int N = sizeof(x_ex) / sizeof(x_ex[0]);
    const int H = sizeof(x_ex[0]) / sizeof(x_ex[0][0]);
    const int W = sizeof(x_ex[0][0]) / sizeof(x_ex[0][0][0]);
    const int C = sizeof(x_ex[0][0][0]) / sizeof(x_ex[0][0][0][0]);

    int i;
    int j;
    int k;
    int l;
    // x,y通道混洗
    /*
        z = tf.stack([x, y], axis=3)  # shape [batch_size, height, width, 2, depth]
        z = tf.transpose(z, [0, 1, 2, 4, 3])
        z = tf.reshape(z, [batch_size, height, width, 2*depth])
        x, y = tf.split(z, num_or_size_splits=2, axis=3)
        ========================================================================
        z = [[1,2,3],[4,5,6]]  ==>  x = [1,2,3] y = [4,5,6]      
        z = [[1,4],[2,5],[3,6]]
        z = [1,4,2,5,3,6]
        x = [1,4,2]   y = [5,3,6]
        ========================================================================
        说白咯就是：
        从x中取一个数放在r_x中,再从y中取一个数放在r_x中，循环直到r_x装满，然后再装r_y
    */
    vector[1][H][W][C] t_x;
    vector[1][H][W][C] t_r_x;
    vector[1][H][W][C] t_y;
    vector[1][H][W][C] t_r_y;
    for (i = 0; i < N; i++)
    {
        t_x = (extern vector[1][H][W][C])x_ex[i];
        t_y = (extern vector[1][H][W][C])y_ex[i];

        strideio(extern vector[0]) x_ex[i][0][0][0], t_x, 1*2, 2*2, (C + 1) / 2;
        strideio(extern vector[0]) x_ex[i][0][0][1], t_y, 1*2, 2*2, C / 2;
        strideio(extern vector[0]) y_ex[i][0][0][0], t_x, 1*2, 2*2, C / 2;
        strideio(extern vector[0]) y_ex[i][0][0][1], t_y, 1*2, 2*2, (C + 1) / 2;
    }
}

def basic_unit(x_ex)
{
    // x[N,H,W,C]==>r_x[N,H,W,C]
    const int N = sizeof(x_ex) / sizeof(x_ex[0]);
    const int H = sizeof(x_ex[0]) / sizeof(x_ex[0][0]);
    const int W = sizeof(x_ex[0][0]) / sizeof(x_ex[0][0][0]);
    const int C = sizeof(x_ex[0][0][0]) / sizeof(x_ex[0][0][0][0]);

    // conv1x1_before
    log("       conv1x1_before");
    extern vector[N][H][W][C] x1_ex;
    extern vector[C][1][1][C] weight;
    extern vector[C] bias;
    EOPConvolution(x1_ex, weight, x_ex, bias, 1, 1, 0, 0);

    // depthwise  depthwise_conv(x, kernel=3, stride=2, activation_fn=None, scope='depthwise')
    log("       depthwise");
    extern vector[N][H][W][C] x2_ex;
    extern vector[C][3][3][C] weight;
    extern vector[C] bias;
    EOPDepthwiseConv(x2_ex, weight, x1_ex, bias, 1, 1, 0, 0);

    // conv1x1_after
    // vector[N][H][W][C] r_y;
    log("       conv1x1_after");
    extern vector[C][1][1][C] weight;
    extern vector[C] bias;
    EOPConvolution(x_ex, weight, x2_ex, bias, 1, 1, 0, 0);
}

def block(x_ex, repeat, out_channels, r_x_ex)
{
    // x[N,H,W,C]==>r_x[N,H/2,W/2,out_channels]
    const int N = sizeof(x_ex) / sizeof(x_ex[0]);
    const int H = sizeof(x_ex[0]) / sizeof(x_ex[0][0]);
    const int W = sizeof(x_ex[0][0]) / sizeof(x_ex[0][0][0]);
    const int C = sizeof(x_ex[0][0][0]) / sizeof(x_ex[0][0][0][0]);

    extern vector[N][H / 2][W / 2][out_channels / 2] x1_ex;
    extern vector[N][H / 2][W / 2][out_channels / 2] y1_ex;

    log("   basic_unit_with_downsampling ...");
    basic_unit_with_downsampling(x_ex, out_channels / 2, x1_ex, y1_ex);

    int i;
    int j;
    int k;
    int l;
    for (i = 0; i < repeat; i++)
    {
        log("   concat_shuffle_split ...");
        concat_shuffle_split(x1_ex, y1_ex);
        log("   basic_unit ...");
        basic_unit(x1_ex);
    }

    // x = tf.concat([x, y], axis=3)
    log("   concating ...");
    vector[1][H / 2][W / 2][out_channels] t_r_x;
    vector[1][H / 2][W / 2][out_channels / 2] t_x1;
    vector[1][H / 2][W / 2][out_channels / 2] t_y1;
    for (i = 0; i < N; i++)
    {
        t_x1 = (extern vector[1][H / 2][W / 2][out_channels / 2]) x1_ex[i];
        t_y1 = (extern vector[1][H / 2][W / 2][out_channels / 2]) y1_ex[i];
        (vector[1][H / 2][W / 2][out_channels / 2]) t_r_x[i][0][0][0] = t_x1;
        (vector[1][H / 2][W / 2][out_channels / 2]) t_r_x[i][0][0][out_channels / 2] = t_y1;
        (extern vector[1][H / 2][W / 2][out_channels]) r_x_ex[i] = t_r_x;
    }
}

// PROGRAME BEGAIN
log("PROGRAME BEGAIN...");
int i = 0;
int j = 0;
int k = 0;
int l = 0;

// normalize
log("normalizing ...");
extern vector[N][224][224][3] X_ex;
extern vector[N][224][224][3] normalized_ex;
{
    vector[1][224][224][3] t_X;
    for (i = 0; i < N; i++)
    {
        t_X = (extern vector[1][224][224][3])X_ex[i];
        t_X = 2 * t_X - 1;
        (extern vector[1][224][224][3]) normalized_ex[i] = t_X;
    }
}

// conv with stride=(2, 2) SAME
log("conv1 ...");
extern vector[N][112][112][24] conv1_ex;
extern vector[24][3][3][3] weight;
extern vector[24] bias;
EOPConvolution(conv1_ex, weight, normalized_ex, bias, 2, 2, 1, 1);

// pooling
log("pooling ...");
extern vector[N][56][56][24] max_pool_ex;
EOPPooling(max_pool_ex, conv1_ex, 3, 3, 2, 2, 1, 1);

log("stage2 ...");
extern vector[N][28][28][48] stage2_ex;
block(max_pool_ex, 3, 48, stage2_ex);

log("stage3 ...");
extern vector[N][14][14][96] stage3_ex;
block(stage2_ex, 7, 96, stage3_ex);

log("stage4 ...");
extern vector[N][7][7][192] stage4_ex;
block(stage3_ex, 3, 192, stage4_ex);

// conv5
log("conv5 ...");
extern vector[N][7][7][1024] conv5_ex;
extern vector[1024][1][1][192] weight;
extern vector[1024] bias;
EOPConvolution(conv5_ex, weight, stage4_ex, bias, 1, 1, 0, 0);

// global average pooling
log("global average pooling ...");
extern vector[N][1][1][1024] global_pool_ex;
EOPPooling(global_pool_ex, conv5_ex, 7, 7, 1, 1, 0, 0);

// flatten and FC
log("flatten ...");
extern vector[N][1024] flattened_ex;
{
    vector[N][1024] flattened;
    vector[N][1][1][1024] t;
    t = global_pool_ex;
    flattened = (vector[N][1024])t;
    flattened_ex = flattened;
}

log("FC ...");
extern vector[N][1000] Y_ex;
extern vector[1000][1024] weight;
extern vector[1000] bias;
EOPFullyConnected(Y_ex, weight, flattened_ex, bias);

log("PROGRAM FINISHED");
