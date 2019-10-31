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

def EOPSoftmax(In) {
    intern decltype(In) t;
    t = In;
    act t, t;
    t /= +t;
    In = t;
}

int rand;

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

def makeanchor(w, h, x0, y0, out) {
    out[0] = x0 - 0.5 * ( w - 1.0);
    out[1] = y0 - 0.5 * ( h - 1.0);
    out[2] = x0 + 0.5 * ( w - 1.0);
    out[3] = y0 + 0.5 * ( h - 1.0);
    out[4] = 0.0;
}   

def gen_anchors(anchors) {
    float t = 16.0;
    vector [4] base_anchor;
    base_anchor[0] = 0.;
    base_anchor[1] = 0.;
    base_anchor[2] = t;
    base_anchor[3] = t;
    int i;
    int j;
    int index;
    float w;
    float h;
    float x0;
    float y0;
    w = base_anchor[2] - base_anchor[0] + 1.0;
    h = base_anchor[3] - base_anchor[1] + 1.0;
    x0 = base_anchor[0] + 0.5 * (w - 1.0);
    y0 = base_anchor[1] + 0.5 * (h - 1.0);
    float size;
    size = w * h;
    float new_size;
    vector [3] ratio;
    vector [4] scale;
    ratio[0] = 0.5;
    ratio[1] = 1.0;
    ratio[2] = 2.0;
    scale[0] = 0.25 * 16;
    scale[1] = 0.5 * 16;
    scale[2] = 1.0 * 16;
    scale[3] = 2.0 * 16;

    float new_w;
    float new_h;

    for (i = 0; i < 3; i++)
        for (j = 0; j < 4; j++) {
            index = i * 4 + j;
            new_size = size / ratio[i];
            sqrt(new_size, new_w);
            new_w = (new_w + 0.5) * scale[j];
            new_h = (new_w / scale[j] * ratio[i] + 0.5) * scale[j];
            makeanchor(new_w, new_h, x0, y0, anchors[index]);
        }
}

def swap(arr, left, right, cols) {
  int i;
  for (i = 0; i < cols; ++i) {
    float temp = arr[left][i];
    arr[left][i] = arr[right][i];
    arr[right][i] = temp;
  }
} 

def partition(pairs, l, h, cols, p) {
  int x0 = pairs[h][0];
  int i = l - 1;
  int j;
  for (j = l; j < h - 1; ++j) {
    int temp = pairs[j][0]; float hacked; get_rand(hacked, 0., 1.);
    /*if (temp <= x0)*/ if (hacked < 0.5) {
      i++;
      swap(pairs, i, j, cols);
    }
  }
  swap(pairs, i+1, h, cols);
  // p = i + 1;  // return p
  p = (l + h) / 2;
}

def sort_score(pairs, rows, cols) {
  // quick sort
  int l = 0;
  int h = rows - 1;
  int stack[10000];
  int top = -1;
  int count = 0;
  stack[++top] = l;
  stack[++top] = h;
  while (top >= 0) {
    h = stack[top--];
    l = stack[top--];
    int p;
    partition(pairs, l, h, cols, p);
    if (p - 1 > l) {
      stack[++top] = l;
      stack[++top] = p - 1;
    }
    // nms just need top 6000
    // save 0.10 - 0.12s
    if (p + 1 < h && p < 6000) {
      stack[++top] = p + 1;
      stack[++top] = h;
    }
    // count += 1;
    // if (count == 10) break;

  }   
}

def BBoxSize(bbox, size) {
  float width = bbox[2] - bbox[0];
  float height = bbox[3] - bbox[1];
  size = (width + 1) * (height + 1);
}

def JaccardOverlap(bbox1, bbox2, overlap) {
  vector[4] intersection;
  intersection[0] = bbox2[0] - bbox1[0];
  intersection[1] = bbox2[1] - bbox1[1];
  intersection[2] = bbox2[2] - bbox1[2];
  intersection[3] = bbox2[3] - bbox1[3];
  int temp;
  temp = (int)(intersection[0] < 0);
  temp = temp || (int)(intersection[1] < 0); 
  temp = temp || (int)(intersection[2] < 0);
  temp = temp || (int)(intersection[3] < 0);
  if (temp) {
    intersection[0] = 0;
    intersection[1] = 0;
    intersection[2] = 0;
    intersection[3] = 0;
  } else {
    float a = bbox1[0];
    float b = bbox2[0];
    intersection[0] = a > b ? a : b;
    a = bbox1[1];
    b = bbox2[1];
    intersection[1] = a > b ? a : b;
    a = bbox1[2];
    b = bbox2[2];
    intersection[2] = a > b ? a : b;
    a = bbox1[3];
    b = bbox2[3];
    intersection[3] = a > b ? a : b;
  }
  float intersect_width = intersection[2] - intersection[0];
  float intersect_height = intersection[3] - intersection[1];
  if (intersect_width * intersect_height > 0) {
    if (intersect_width > 0) {
      float intersect_size = intersect_width * intersect_height;
      float bbox1_size;
      BBoxSize(bbox1, bbox1_size);
      float bbox2_size;
      BBoxSize(bbox2, bbox2_size);
      overlap = intersect_size / (bbox1_size + bbox2_size - intersect_size);
    } else {
      overlap = 0;
    }
  } else {
    overlap = 0;
  }
}

def EOPProposal(cls, bbox_pred, im_info, roi) {

    print("proposal");
    int in_data_shape_1 = 24;
    const int Height = 38;
    const int Width = 61;
    const int Num_Anchors = 12 * 38 * 61;
    int num_anchors = in_data_shape_1 / 2 * 38 * 61;
    int i;
    int j;
    int k;
    int index;

    int rpn_pre_num_top_n = 6000;
    int rpn_post_num_top_n = 300;
    
    vector [1][12][Height][Width] scores;
    // load scores
    strideio (vector[0])scores, (extern vector[0])cls[0][12][0][0], Num_Anchors, 0, 1;

    vector [12][5] anchors;
    gen_anchors(anchors);

    vector [12 * Height * Width][5] workspace;
    (decltype(anchors))workspace = anchors;


    for(i = 0; i < 12; i++)
        for(j = 0; j < Height; j++)
            for(k = 0; k < Width; k++) {
                index = j * (Width * 12) + k * 12 + i;
                workspace[index][0] = workspace[i][0] + k * 16.0;
                workspace[index][1] = workspace[i][1] + j * 16.0;
                workspace[index][2] = workspace[i][2] + k * 16.0;
                workspace[index][3] = workspace[i][3] + j * 16.0;
                workspace[index][4] = scores[0][i][j][k];
            }

    // bbox prediction
    // TODO

    // copy score
    vector [12 * 38 * 61][2] score_order;
    float tmp;
    print("copy score");
    for (i = 0; i < num_anchors; i++) {
        // get_rand(tmp, 0., 10.); 
        score_order[i][0] = workspace[i][4];
        score_order[i][1] = i;
    }
    sort_score(score_order, Num_Anchors , 2);
    print("sorted");

    // pre nms
    // reorder

    vector [6000][5] order;
    for (i = 0; i < 6000; i++)
        for (j = 0; j < 5; j++) {
            index = score_order[i][j];
            order[i][j] = workspace[index][j];
        }

    // NMS
    int count = 0;
    float iou;
    const float num_thresh = 0.5;
    vector [6000] suppressed;
    vector [300][5] out_roi;
    vector [300][1] out_score;
    print("NMS");
    for (i = 0; i < 6000; i++) {
        if (suppressed[i] > 0.0) continue;
        out_roi[count][0] = 0;
        out_roi[count][1] = order[i][0]; 
        out_roi[count][2] = order[i][1]; 
        out_roi[count][3] = order[i][2]; 
        out_roi[count][4] = order[i][3]; 
        out_score[count][0] = order[i][4];
        count = count + 1;
        if (count >= 300)
            break;
        for (j = i + 1; j < 6000; j++) {
            JaccardOverlap(order[i], order[j], iou);
            if (iou > num_thresh)
                suppressed[j] = 1.0;
        }
    }
    strideio (extern vector[0])roi, out_roi, 1500, 0, 1;
    // strideio (extern vector[0])score, out_score, 300, 0, 1;
}

// boader
def min_max(x, min_b, max_b, ret) {
    ret = x;
    if (ret < 0) 
        ret = x;
    if (ret > max_b)
        ret = max_b;
}

// roi = [index, x1, y1, x2, y2]
def EOPRoipooling(input, roi, output) {
    const int Height = 38;
    const int Width = 61;
    const int Roi_Channel = 1024;
    const int Stride_A = Roi_Channel - 1;
    const float sp = 1.0 / 16.0;
    const int pooled_height = 14;
    const int pooled_width = 14;
    // extern vector [1][Height][Width][Roi_Channel] input; // [1, 1024, Height, Width]
    // extern vector [300][14][14][Roi_Channel] output; // output = [300, c, 14, 14]
    // extern vector [300][5] roi;
    vector [5] bbox;
    int num_roi = 300;

    // for roi
    int roi_start_w;
    int roi_start_h;
    int roi_end_w;
    int roi_end_h;
    int roi_height;
    int roi_width;
    float bin_size_h;
    float bin_size_w;
    int hstart;
    int wstart;
    int hend;
    int wend;

    int i;
    int j;
    int k;
    int ph;
    int pw;

    // each point in 14 * 14, 
    // feature map 3 * 3, maxpooling it
    const int AA = 3 * 1024;
    const int BB = 61 * 1024;
    const int CC = 14 * 14 * 1024;

    for(i = 0; i < num_roi; i++) {
        bbox = roi[i];
        // print("each roi");
        roi_start_w = (int)(bbox[1] * sp);
        roi_start_h = (int)(bbox[2] * sp);
        roi_end_w = (int)(bbox[3] * sp);
        roi_end_h = (int)(bbox[4] * sp);

        roi_height = roi_end_h - roi_start_h + 1;
        roi_width = roi_end_w - roi_end_h + 1;
        if (roi_height < 1)
            roi_height = 1;
        if (roi_width < 1)
            roi_width = 1;

        bin_size_h = roi_height / pooled_height;
        bin_size_w = roi_width / pooled_width;
        // for channel
        // for(j = 0; j < Roi_Channel; j++) {
            // print("each channel");
            // for each feature map, every point, roipooling
            vector [14][14][1024] single_out;
            for(ph = 0; ph < pooled_height; ph++)
                for(pw = 0; pw < pooled_width; pw++) {
                    hstart = (int)((float)(ph) * bin_size_h);
                    wstart = (int)((float)(pw) * bin_size_w);
                    hend = (int)((float)(ph + 1) * bin_size_h); 
                    wend = (int)((float)(pw + 1) * bin_size_h);

                    min_max(hstart + roi_start_h, 0, Height, hstart);
                    min_max(hend + roi_start_h, 0, Height, hend);
                    min_max(wstart + roi_start_w, 0, Width, wstart);
                    min_max(wend + roi_start_w, 0, Width, wend);

                    hstart = 0;
                    // make it to 0
                    // buf, addr, a, w-a, a

                    // always select 16 * 16 from feature map
                    vector [1][1][1024] out;
                    vector [3][3][1024] buf;
                    strideio (vector[0])buf[0][0][0], (extern vector[0])input[0][hstart][0][0], AA, BB, 2;
                    pool out, buf, 3, 3, 1, 1, 0, 0;
                    single_out[ph][pw] = out[0][0];
                }

            // [14][14][1] -> [i][14][14][j]
            strideio (extern vector[0])output[i][0][0][0], single_out, CC, 0, 1;
        // }
    }
}

//# // EOPConvolution(extern vector[NHWC/HWC] dest, extern vector[NHWC] kernel, extern vector[NHWC/HWC] input, \ 
//# //                extern vector[C] bias, const int stride_x, const int stride_y, const int pad_x, const int pad_y);
//  extern vector[600][966][3] data;
extern vector[100][966][3] data;
extern vector[64][7][7][3] conv0_weight;
extern vector[50][483][64] conv0_;
EOPConvolution(conv0_, conv0_weight, data, null, 2, 2, 3, 3);
EOPConvolution(conv0_, conv0_weight, data, null, 2, 2, 3, 3);
EOPConvolution(conv0_, conv0_weight, data, null, 2, 2, 3, 3);
EOPConvolution(conv0_, conv0_weight, data, null, 2, 2, 3, 3);
EOPConvolution(conv0_, conv0_weight, data, null, 2, 2, 3, 3);
EOPConvolution(conv0_, conv0_weight, data, null, 2, 2, 3, 3);

extern vector[300][483][64] conv0;
// EOPConvolution(conv0, conv0_weight, data, null, 1, 1, 3, 3);
//# // EOPPooling(extern vector[NHWC/HWC] dest, extern vector[NHWC/HWC] input, const int kernel_x, const int kernel_y, \ 
//# //            const int stride_x, const int stride_y, const int pad_x, const int pad_y);
extern vector[150][242][64] pool0;
EOPPooling(pool0, conv0, 3, 3, 2, 2, 1, 1);
//# 
// stage1_unit1
extern vector[64][1][1][64] stage1_unit1_conv1_weight;
extern vector[1][150][242][64] stage1_unit1_conv1;
EOPConvolution(stage1_unit1_conv1, stage1_unit1_conv1_weight, pool0, null, 1, 1, 0, 0);
extern vector[64][1][1][64] stage1_unit1_conv2_weight;
extern vector[1][150][242][64] stage1_unit1_conv2;
EOPConvolution(stage1_unit1_conv2, stage1_unit1_conv2_weight, stage1_unit1_conv1, null, 1, 1, 1, 1);
extern vector[256][1][1][64] stage1_unit1_conv3_weight;
extern vector[1][150][242][256] stage1_unit1_conv3;
EOPConvolution(stage1_unit1_conv3, stage1_unit1_conv3_weight, stage1_unit1_conv2, null, 1, 1, 0, 0);
extern vector[256][1][1][64] stage1_unit1_sc_weight;
extern vector[1][150][242][256] stage1_unit1_sc;
EOPConvolution(stage1_unit1_sc, stage1_unit1_sc_weight, pool0, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage1_unit1_conv3 + stage1_unit1_sc = stage1_unit1_plus;
extern vector[1][150][242][256] stage1_unit1_plus;
ElementWiseADD(stage1_unit1_conv3, stage1_unit1_sc, stage1_unit1_plus);

// stage1_unit2
extern vector[64][1][1][256] stage1_unit2_conv1_weight;
extern vector[1][150][242][64] stage1_unit2_conv1;
EOPConvolution(stage1_unit2_conv1, stage1_unit2_conv1_weight, stage1_unit1_plus, null, 1, 1, 0, 0);
extern vector[64][1][1][64] stage1_unit2_conv2_weight;
extern vector[1][150][242][64] stage1_unit2_conv2;
EOPConvolution(stage1_unit2_conv2, stage1_unit2_conv2_weight, stage1_unit2_conv1, null, 1, 1, 1, 1);
extern vector[256][1][1][64] stage1_unit2_conv3_weight;
extern vector[1][150][242][256] stage1_unit2_conv3;
EOPConvolution(stage1_unit2_conv3, stage1_unit2_conv3_weight, stage1_unit2_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage1_unit2_conv3 + stage1_unit1_plus = stage1_unit2_plus;
extern vector[1][150][242][256] stage1_unit2_plus;
ElementWiseADD(stage1_unit2_conv3, stage1_unit1_plus, stage1_unit2_plus);

// stage1_unit3
extern vector[64][1][1][256] stage1_unit3_conv1_weight;
extern vector[1][150][242][64] stage1_unit3_conv1;
EOPConvolution(stage1_unit3_conv1, stage1_unit3_conv1_weight, stage1_unit2_plus, null, 1, 1, 0, 0);
extern vector[64][1][1][64] stage1_unit3_conv2_weight;
extern vector[1][150][242][64] stage1_unit3_conv2;
EOPConvolution(stage1_unit3_conv2, stage1_unit3_conv2_weight, stage1_unit3_conv1, null, 1, 1, 1, 1);
extern vector[256][1][1][64] stage1_unit3_conv3_weight;
extern vector[1][150][242][256] stage1_unit3_conv3;
EOPConvolution(stage1_unit3_conv3, stage1_unit3_conv3_weight, stage1_unit3_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage1_unit3_conv3 + stage1_unit2_plus = stage1_unit3_plus;
extern vector[1][150][242][256] stage1_unit3_plus;
ElementWiseADD(stage1_unit3_conv3, stage1_unit2_plus, stage1_unit3_plus);

// stage2_unit1
extern vector[128][1][1][256] stage2_unit1_conv1_weight;
extern vector[1][150][242][128] stage2_unit1_conv1;
EOPConvolution(stage2_unit1_conv1, stage2_unit1_conv1_weight, stage1_unit3_plus, null, 1, 1, 0, 0);
extern vector[128][1][1][128] stage2_unit1_conv2_weight;
extern vector[1][75][121][128] stage2_unit1_conv2;
EOPConvolution(stage2_unit1_conv2, stage2_unit1_conv2_weight, stage2_unit1_conv1, null, 2, 2, 1, 1);
extern vector[512][1][1][128] stage2_unit1_conv3_weight;
extern vector[1][75][121][512] stage2_unit1_conv3;
EOPConvolution(stage2_unit1_conv3, stage2_unit1_conv3_weight, stage2_unit1_conv2, null, 1, 1, 0, 0);
extern vector[512][1][1][256] stage2_unit1_sc_weight;
extern vector[1][75][121][512] stage2_unit1_sc;
EOPConvolution(stage2_unit1_sc, stage2_unit1_sc_weight, stage1_unit3_plus, null, 2, 2, 0, 0);
// EOPElementWiseADD: stage2_unit1_conv3 + stage2_unit1_sc = stage2_unit1_plus;
extern vector[1][75][121][512] stage2_unit1_plus;
ElementWiseADD(stage2_unit1_conv3, stage2_unit1_sc, stage2_unit1_plus);

// stage2_unit2
extern vector[128][1][1][512] stage2_unit2_conv1_weight;
extern vector[1][75][121][128] stage2_unit2_conv1;
EOPConvolution(stage2_unit2_conv1, stage2_unit2_conv1_weight, stage2_unit1_plus, null, 1, 1, 0, 0);
extern vector[128][1][1][128] stage2_unit2_conv2_weight;
extern vector[1][75][121][128] stage2_unit2_conv2;
EOPConvolution(stage2_unit2_conv2, stage2_unit2_conv2_weight, stage2_unit2_conv1, null, 1, 1, 1, 1);
extern vector[512][1][1][128] stage2_unit2_conv3_weight;
extern vector[1][75][121][512] stage2_unit2_conv3;
EOPConvolution(stage2_unit2_conv3, stage2_unit2_conv3_weight, stage2_unit2_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage2_unit2_conv3 + stage2_unit1_plus = stage2_unit2_plus;
extern vector[1][75][121][512] stage2_unit2_plus;
ElementWiseADD(stage2_unit2_conv3, stage2_unit1_plus, stage2_unit2_plus);

// stage2_unit3
extern vector[128][1][1][512] stage2_unit3_conv1_weight;
extern vector[1][75][121][128] stage2_unit3_conv1;
EOPConvolution(stage2_unit3_conv1, stage2_unit3_conv1_weight, stage2_unit2_plus, null, 1, 1, 0, 0);
extern vector[128][1][1][128] stage2_unit3_conv2_weight;
extern vector[1][75][121][128] stage2_unit3_conv2;
EOPConvolution(stage2_unit3_conv2, stage2_unit3_conv2_weight, stage2_unit3_conv1, null, 1, 1, 1, 1);
extern vector[512][1][1][128] stage2_unit3_conv3_weight;
extern vector[1][75][121][512] stage2_unit3_conv3;
EOPConvolution(stage2_unit3_conv3, stage2_unit3_conv3_weight, stage2_unit3_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage2_unit3_conv3 + stage2_unit2_plus = stage2_unit3_plus;
extern vector[1][75][121][512] stage2_unit3_plus;
ElementWiseADD(stage2_unit3_conv3, stage2_unit2_plus, stage2_unit3_plus);

// stage2_unit4
extern vector[128][1][1][512] stage2_unit4_conv1_weight;
extern vector[1][75][121][128] stage2_unit4_conv1;
EOPConvolution(stage2_unit4_conv1, stage2_unit4_conv1_weight, stage2_unit3_plus, null, 1, 1, 0, 0);
extern vector[128][1][1][128] stage2_unit4_conv2_weight;
extern vector[1][75][121][128] stage2_unit4_conv2;
EOPConvolution(stage2_unit4_conv2, stage2_unit4_conv2_weight, stage2_unit4_conv1, null, 1, 1, 1, 1);
extern vector[512][1][1][128] stage2_unit4_conv3_weight;
extern vector[1][75][121][512] stage2_unit4_conv3;
EOPConvolution(stage2_unit4_conv3, stage2_unit4_conv3_weight, stage2_unit4_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage2_unit4_conv3 + stage2_unit3_plus = stage2_unit4_plus;
extern vector[1][75][121][512] stage2_unit4_plus;
ElementWiseADD(stage2_unit4_conv3, stage2_unit3_plus, stage2_unit4_plus);

// stage3_unit1
extern vector[256][1][1][512] stage3_unit1_conv1_weight;
extern vector[1][75][121][256] stage3_unit1_conv1;
EOPConvolution(stage3_unit1_conv1, stage3_unit1_conv1_weight, stage2_unit4_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit1_conv2_weight;
extern vector[1][38][61][256] stage3_unit1_conv2;
EOPConvolution(stage3_unit1_conv2, stage3_unit1_conv2_weight, stage3_unit1_conv1, null, 2, 2, 1, 1);
extern vector[1024][1][1][256] stage3_unit1_conv3_weight;
extern vector[1][38][61][1024] stage3_unit1_conv3;
EOPConvolution(stage3_unit1_conv3, stage3_unit1_conv3_weight, stage3_unit1_conv2, null, 1, 1, 0, 0);
extern vector[1024][1][1][512] stage3_unit1_sc_weight;
extern vector[1][38][61][1024] stage3_unit1_sc;
EOPConvolution(stage3_unit1_sc, stage3_unit1_sc_weight, stage2_unit4_plus, null, 2, 2, 0, 0);
// EOPElementWiseADD: stage3_unit1_conv3 + stage3_unit1_sc = stage3_unit1_plus;
extern vector[1][38][61][1024] stage3_unit1_plus;
ElementWiseADD(stage3_unit1_conv3, stage3_unit1_sc, stage3_unit1_plus);

// stage3_unit2
extern vector[256][1][1][1024] stage3_unit2_conv1_weight;
extern vector[1][38][61][256] stage3_unit2_conv1;
EOPConvolution(stage3_unit2_conv1, stage3_unit2_conv1_weight, stage3_unit1_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit2_conv2_weight;
extern vector[1][38][61][256] stage3_unit2_conv2;
EOPConvolution(stage3_unit2_conv2, stage3_unit2_conv2_weight, stage3_unit2_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit2_conv3_weight;
extern vector[1][38][61][1024] stage3_unit2_conv3;
EOPConvolution(stage3_unit2_conv3, stage3_unit2_conv3_weight, stage3_unit2_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit2_conv3 + stage3_unit1_plus = stage3_unit2_plus;
extern vector[1][38][61][1024] stage3_unit2_plus;
ElementWiseADD(stage3_unit2_conv3, stage3_unit1_plus, stage3_unit2_plus);

// stage3_unit3
extern vector[256][1][1][1024] stage3_unit3_conv1_weight;
extern vector[1][38][61][256] stage3_unit3_conv1;
EOPConvolution(stage3_unit3_conv1, stage3_unit3_conv1_weight, stage3_unit2_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit3_conv2_weight;
extern vector[1][38][61][256] stage3_unit3_conv2;
EOPConvolution(stage3_unit3_conv2, stage3_unit3_conv2_weight, stage3_unit3_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit3_conv3_weight;
extern vector[1][38][61][1024] stage3_unit3_conv3;
EOPConvolution(stage3_unit3_conv3, stage3_unit3_conv3_weight, stage3_unit3_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit3_conv3 + stage3_unit2_plus = stage3_unit3_plus;
extern vector[1][38][61][1024] stage3_unit3_plus;
ElementWiseADD(stage3_unit3_conv3, stage3_unit2_plus, stage3_unit3_plus);

// stage3_unit4
extern vector[256][1][1][1024] stage3_unit4_conv1_weight;
extern vector[1][38][61][256] stage3_unit4_conv1;
EOPConvolution(stage3_unit4_conv1, stage3_unit4_conv1_weight, stage3_unit3_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit4_conv2_weight;
extern vector[1][38][61][256] stage3_unit4_conv2;
EOPConvolution(stage3_unit4_conv2, stage3_unit4_conv2_weight, stage3_unit4_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit4_conv3_weight;
extern vector[1][38][61][1024] stage3_unit4_conv3;
EOPConvolution(stage3_unit4_conv3, stage3_unit4_conv3_weight, stage3_unit4_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit4_conv3 + stage3_unit3_plus = stage3_unit4_plus;
extern vector[1][38][61][1024] stage3_unit4_plus;
ElementWiseADD(stage3_unit4_conv3, stage3_unit3_plus, stage3_unit4_plus);

// stage3_unit5
extern vector[256][1][1][1024] stage3_unit5_conv1_weight;
extern vector[1][38][61][256] stage3_unit5_conv1;
EOPConvolution(stage3_unit5_conv1, stage3_unit5_conv1_weight, stage3_unit4_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit5_conv2_weight;
extern vector[1][38][61][256] stage3_unit5_conv2;
EOPConvolution(stage3_unit5_conv2, stage3_unit5_conv2_weight, stage3_unit5_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit5_conv3_weight;
extern vector[1][38][61][1024] stage3_unit5_conv3;
EOPConvolution(stage3_unit5_conv3, stage3_unit5_conv3_weight, stage3_unit5_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit5_conv3 + stage3_unit4_plus = stage3_unit5_plus;
extern vector[1][38][61][1024] stage3_unit5_plus;
ElementWiseADD(stage3_unit5_conv3, stage3_unit4_plus, stage3_unit5_plus);

// stage3_unit6
extern vector[256][1][1][1024] stage3_unit6_conv1_weight;
extern vector[1][38][61][256] stage3_unit6_conv1;
EOPConvolution(stage3_unit6_conv1, stage3_unit6_conv1_weight, stage3_unit5_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit6_conv2_weight;
extern vector[1][38][61][256] stage3_unit6_conv2;
EOPConvolution(stage3_unit6_conv2, stage3_unit6_conv2_weight, stage3_unit6_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit6_conv3_weight;
extern vector[1][38][61][1024] stage3_unit6_conv3;
EOPConvolution(stage3_unit6_conv3, stage3_unit6_conv3_weight, stage3_unit6_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit6_conv3 + stage3_unit5_plus = stage3_unit6_plus;
extern vector[1][38][61][1024] stage3_unit6_plus;
ElementWiseADD(stage3_unit6_conv3, stage3_unit5_plus, stage3_unit6_plus);

// stage3_unit7
extern vector[256][1][1][1024] stage3_unit7_conv1_weight;
extern vector[1][38][61][256] stage3_unit7_conv1;
EOPConvolution(stage3_unit7_conv1, stage3_unit7_conv1_weight, stage3_unit6_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit7_conv2_weight;
extern vector[1][38][61][256] stage3_unit7_conv2;
EOPConvolution(stage3_unit7_conv2, stage3_unit7_conv2_weight, stage3_unit7_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit7_conv3_weight;
extern vector[1][38][61][1024] stage3_unit7_conv3;
EOPConvolution(stage3_unit7_conv3, stage3_unit7_conv3_weight, stage3_unit7_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit7_conv3 + stage3_unit6_plus = stage3_unit7_plus;
extern vector[1][38][61][1024] stage3_unit7_plus;
ElementWiseADD(stage3_unit7_conv3, stage3_unit6_plus, stage3_unit7_plus);

// stage3_unit8
extern vector[256][1][1][1024] stage3_unit8_conv1_weight;
extern vector[1][38][61][256] stage3_unit8_conv1;
EOPConvolution(stage3_unit8_conv1, stage3_unit8_conv1_weight, stage3_unit7_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit8_conv2_weight;
extern vector[1][38][61][256] stage3_unit8_conv2;
EOPConvolution(stage3_unit8_conv2, stage3_unit8_conv2_weight, stage3_unit8_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit8_conv3_weight;
extern vector[1][38][61][1024] stage3_unit8_conv3;
EOPConvolution(stage3_unit8_conv3, stage3_unit8_conv3_weight, stage3_unit8_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit8_conv3 + stage3_unit7_plus = stage3_unit8_plus;
extern vector[1][38][61][1024] stage3_unit8_plus;
ElementWiseADD(stage3_unit8_conv3, stage3_unit7_plus, stage3_unit8_plus);

// stage3_unit9
extern vector[256][1][1][1024] stage3_unit9_conv1_weight;
extern vector[1][38][61][256] stage3_unit9_conv1;
EOPConvolution(stage3_unit9_conv1, stage3_unit9_conv1_weight, stage3_unit8_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit9_conv2_weight;
extern vector[1][38][61][256] stage3_unit9_conv2;
EOPConvolution(stage3_unit9_conv2, stage3_unit9_conv2_weight, stage3_unit9_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit9_conv3_weight;
extern vector[1][38][61][1024] stage3_unit9_conv3;
EOPConvolution(stage3_unit9_conv3, stage3_unit9_conv3_weight, stage3_unit9_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit9_conv3 + stage3_unit8_plus = stage3_unit9_plus;
extern vector[1][38][61][1024] stage3_unit9_plus;
ElementWiseADD(stage3_unit9_conv3, stage3_unit8_plus, stage3_unit9_plus);

// stage3_unit10
extern vector[256][1][1][1024] stage3_unit10_conv1_weight;
extern vector[1][38][61][256] stage3_unit10_conv1;
EOPConvolution(stage3_unit10_conv1, stage3_unit10_conv1_weight, stage3_unit9_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit10_conv2_weight;
extern vector[1][38][61][256] stage3_unit10_conv2;
EOPConvolution(stage3_unit10_conv2, stage3_unit10_conv2_weight, stage3_unit10_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit10_conv3_weight;
extern vector[1][38][61][1024] stage3_unit10_conv3;
EOPConvolution(stage3_unit10_conv3, stage3_unit10_conv3_weight, stage3_unit10_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit10_conv3 + stage3_unit9_plus = stage3_unit10_plus;
extern vector[1][38][61][1024] stage3_unit10_plus;
ElementWiseADD(stage3_unit10_conv3, stage3_unit9_plus, stage3_unit10_plus);

// stage3_unit11
extern vector[256][1][1][1024] stage3_unit11_conv1_weight;
extern vector[1][38][61][256] stage3_unit11_conv1;
EOPConvolution(stage3_unit11_conv1, stage3_unit11_conv1_weight, stage3_unit10_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit11_conv2_weight;
extern vector[1][38][61][256] stage3_unit11_conv2;
EOPConvolution(stage3_unit11_conv2, stage3_unit11_conv2_weight, stage3_unit11_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit11_conv3_weight;
extern vector[1][38][61][1024] stage3_unit11_conv3;
EOPConvolution(stage3_unit11_conv3, stage3_unit11_conv3_weight, stage3_unit11_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit11_conv3 + stage3_unit10_plus = stage3_unit11_plus;
extern vector[1][38][61][1024] stage3_unit11_plus;
ElementWiseADD(stage3_unit11_conv3, stage3_unit10_plus, stage3_unit11_plus);

// stage3_unit12
extern vector[256][1][1][1024] stage3_unit12_conv1_weight;
extern vector[1][38][61][256] stage3_unit12_conv1;
EOPConvolution(stage3_unit12_conv1, stage3_unit12_conv1_weight, stage3_unit11_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit12_conv2_weight;
extern vector[1][38][61][256] stage3_unit12_conv2;
EOPConvolution(stage3_unit12_conv2, stage3_unit12_conv2_weight, stage3_unit12_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit12_conv3_weight;
extern vector[1][38][61][1024] stage3_unit12_conv3;
EOPConvolution(stage3_unit12_conv3, stage3_unit12_conv3_weight, stage3_unit12_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit12_conv3 + stage3_unit11_plus = stage3_unit12_plus;
extern vector[1][38][61][1024] stage3_unit12_plus;
ElementWiseADD(stage3_unit12_conv3, stage3_unit11_plus, stage3_unit12_plus);

// stage3_unit13
extern vector[256][1][1][1024] stage3_unit13_conv1_weight;
extern vector[1][38][61][256] stage3_unit13_conv1;
EOPConvolution(stage3_unit13_conv1, stage3_unit13_conv1_weight, stage3_unit12_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit13_conv2_weight;
extern vector[1][38][61][256] stage3_unit13_conv2;
EOPConvolution(stage3_unit13_conv2, stage3_unit13_conv2_weight, stage3_unit13_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit13_conv3_weight;
extern vector[1][38][61][1024] stage3_unit13_conv3;
EOPConvolution(stage3_unit13_conv3, stage3_unit13_conv3_weight, stage3_unit13_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit13_conv3 + stage3_unit12_plus = stage3_unit13_plus;
extern vector[1][38][61][1024] stage3_unit13_plus;
ElementWiseADD(stage3_unit13_conv3, stage3_unit12_plus, stage3_unit13_plus);

// stage3_unit14
extern vector[256][1][1][1024] stage3_unit14_conv1_weight;
extern vector[1][38][61][256] stage3_unit14_conv1;
EOPConvolution(stage3_unit14_conv1, stage3_unit14_conv1_weight, stage3_unit13_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit14_conv2_weight;
extern vector[1][38][61][256] stage3_unit14_conv2;
EOPConvolution(stage3_unit14_conv2, stage3_unit14_conv2_weight, stage3_unit14_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit14_conv3_weight;
extern vector[1][38][61][1024] stage3_unit14_conv3;
EOPConvolution(stage3_unit14_conv3, stage3_unit14_conv3_weight, stage3_unit14_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit14_conv3 + stage3_unit13_plus = stage3_unit14_plus;
extern vector[1][38][61][1024] stage3_unit14_plus;
ElementWiseADD(stage3_unit14_conv3, stage3_unit13_plus, stage3_unit14_plus);

// stage3_unit15
extern vector[256][1][1][1024] stage3_unit15_conv1_weight;
extern vector[1][38][61][256] stage3_unit15_conv1;
EOPConvolution(stage3_unit15_conv1, stage3_unit15_conv1_weight, stage3_unit14_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit15_conv2_weight;
extern vector[1][38][61][256] stage3_unit15_conv2;
EOPConvolution(stage3_unit15_conv2, stage3_unit15_conv2_weight, stage3_unit15_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit15_conv3_weight;
extern vector[1][38][61][1024] stage3_unit15_conv3;
EOPConvolution(stage3_unit15_conv3, stage3_unit15_conv3_weight, stage3_unit15_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit15_conv3 + stage3_unit14_plus = stage3_unit15_plus;
extern vector[1][38][61][1024] stage3_unit15_plus;
ElementWiseADD(stage3_unit15_conv3, stage3_unit14_plus, stage3_unit15_plus);

// stage3_unit16
extern vector[256][1][1][1024] stage3_unit16_conv1_weight;
extern vector[1][38][61][256] stage3_unit16_conv1;
EOPConvolution(stage3_unit16_conv1, stage3_unit16_conv1_weight, stage3_unit15_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit16_conv2_weight;
extern vector[1][38][61][256] stage3_unit16_conv2;
EOPConvolution(stage3_unit16_conv2, stage3_unit16_conv2_weight, stage3_unit16_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit16_conv3_weight;
extern vector[1][38][61][1024] stage3_unit16_conv3;
EOPConvolution(stage3_unit16_conv3, stage3_unit16_conv3_weight, stage3_unit16_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit16_conv3 + stage3_unit15_plus = stage3_unit16_plus;
extern vector[1][38][61][1024] stage3_unit16_plus;
ElementWiseADD(stage3_unit16_conv3, stage3_unit15_plus, stage3_unit16_plus);

// stage3_unit17
extern vector[256][1][1][1024] stage3_unit17_conv1_weight;
extern vector[1][38][61][256] stage3_unit17_conv1;
EOPConvolution(stage3_unit17_conv1, stage3_unit17_conv1_weight, stage3_unit16_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit17_conv2_weight;
extern vector[1][38][61][256] stage3_unit17_conv2;
EOPConvolution(stage3_unit17_conv2, stage3_unit17_conv2_weight, stage3_unit17_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit17_conv3_weight;
extern vector[1][38][61][1024] stage3_unit17_conv3;
EOPConvolution(stage3_unit17_conv3, stage3_unit17_conv3_weight, stage3_unit17_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit17_conv3 + stage3_unit16_plus = stage3_unit17_plus;
extern vector[1][38][61][1024] stage3_unit17_plus;
ElementWiseADD(stage3_unit17_conv3, stage3_unit16_plus, stage3_unit17_plus);

// stage3_unit18
extern vector[256][1][1][1024] stage3_unit18_conv1_weight;
extern vector[1][38][61][256] stage3_unit18_conv1;
EOPConvolution(stage3_unit18_conv1, stage3_unit18_conv1_weight, stage3_unit17_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit18_conv2_weight;
extern vector[1][38][61][256] stage3_unit18_conv2;
EOPConvolution(stage3_unit18_conv2, stage3_unit18_conv2_weight, stage3_unit18_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit18_conv3_weight;
extern vector[1][38][61][1024] stage3_unit18_conv3;
EOPConvolution(stage3_unit18_conv3, stage3_unit18_conv3_weight, stage3_unit18_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit18_conv3 + stage3_unit17_plus = stage3_unit18_plus;
extern vector[1][38][61][1024] stage3_unit18_plus;
ElementWiseADD(stage3_unit18_conv3, stage3_unit17_plus, stage3_unit18_plus);

// stage3_unit19
extern vector[256][1][1][1024] stage3_unit19_conv1_weight;
extern vector[1][38][61][256] stage3_unit19_conv1;
EOPConvolution(stage3_unit19_conv1, stage3_unit19_conv1_weight, stage3_unit18_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit19_conv2_weight;
extern vector[1][38][61][256] stage3_unit19_conv2;
EOPConvolution(stage3_unit19_conv2, stage3_unit19_conv2_weight, stage3_unit19_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit19_conv3_weight;
extern vector[1][38][61][1024] stage3_unit19_conv3;
EOPConvolution(stage3_unit19_conv3, stage3_unit19_conv3_weight, stage3_unit19_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit19_conv3 + stage3_unit18_plus = stage3_unit19_plus;
extern vector[1][38][61][1024] stage3_unit19_plus;
ElementWiseADD(stage3_unit19_conv3, stage3_unit18_plus, stage3_unit19_plus);

// stage3_unit20
extern vector[256][1][1][1024] stage3_unit20_conv1_weight;
extern vector[1][38][61][256] stage3_unit20_conv1;
EOPConvolution(stage3_unit20_conv1, stage3_unit20_conv1_weight, stage3_unit19_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit20_conv2_weight;
extern vector[1][38][61][256] stage3_unit20_conv2;
EOPConvolution(stage3_unit20_conv2, stage3_unit20_conv2_weight, stage3_unit20_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit20_conv3_weight;
extern vector[1][38][61][1024] stage3_unit20_conv3;
EOPConvolution(stage3_unit20_conv3, stage3_unit20_conv3_weight, stage3_unit20_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit20_conv3 + stage3_unit19_plus = stage3_unit20_plus;
extern vector[1][38][61][1024] stage3_unit20_plus;
ElementWiseADD(stage3_unit20_conv3, stage3_unit19_plus, stage3_unit20_plus);

// stage3_unit21
extern vector[256][1][1][1024] stage3_unit21_conv1_weight;
extern vector[1][38][61][256] stage3_unit21_conv1;
EOPConvolution(stage3_unit21_conv1, stage3_unit21_conv1_weight, stage3_unit20_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit21_conv2_weight;
extern vector[1][38][61][256] stage3_unit21_conv2;
EOPConvolution(stage3_unit21_conv2, stage3_unit21_conv2_weight, stage3_unit21_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit21_conv3_weight;
extern vector[1][38][61][1024] stage3_unit21_conv3;
EOPConvolution(stage3_unit21_conv3, stage3_unit21_conv3_weight, stage3_unit21_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit21_conv3 + stage3_unit20_plus = stage3_unit21_plus;
extern vector[1][38][61][1024] stage3_unit21_plus;
ElementWiseADD(stage3_unit21_conv3, stage3_unit20_plus, stage3_unit21_plus);

// stage3_unit22
extern vector[256][1][1][1024] stage3_unit22_conv1_weight;
extern vector[1][38][61][256] stage3_unit22_conv1;
EOPConvolution(stage3_unit22_conv1, stage3_unit22_conv1_weight, stage3_unit21_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit22_conv2_weight;
extern vector[1][38][61][256] stage3_unit22_conv2;
EOPConvolution(stage3_unit22_conv2, stage3_unit22_conv2_weight, stage3_unit22_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit22_conv3_weight;
extern vector[1][38][61][1024] stage3_unit22_conv3;
EOPConvolution(stage3_unit22_conv3, stage3_unit22_conv3_weight, stage3_unit22_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit22_conv3 + stage3_unit21_plus = stage3_unit22_plus;
extern vector[1][38][61][1024] stage3_unit22_plus;
ElementWiseADD(stage3_unit22_conv3, stage3_unit21_plus, stage3_unit22_plus);

// stage3_unit23
extern vector[256][1][1][1024] stage3_unit23_conv1_weight;
extern vector[1][38][61][256] stage3_unit23_conv1;
EOPConvolution(stage3_unit23_conv1, stage3_unit23_conv1_weight, stage3_unit22_plus, null, 1, 1, 0, 0);
extern vector[256][1][1][256] stage3_unit23_conv2_weight;
extern vector[1][38][61][256] stage3_unit23_conv2;
EOPConvolution(stage3_unit23_conv2, stage3_unit23_conv2_weight, stage3_unit23_conv1, null, 1, 1, 1, 1);
extern vector[1024][1][1][256] stage3_unit23_conv3_weight;
extern vector[1][38][61][1024] stage3_unit23_conv3;
EOPConvolution(stage3_unit23_conv3, stage3_unit23_conv3_weight, stage3_unit23_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage3_unit23_conv3 + stage3_unit22_plus = stage3_unit23_plus;
extern vector[1][38][61][1024] stage3_unit23_plus;
ElementWiseADD(stage3_unit23_conv3, stage3_unit22_plus, stage3_unit23_plus);

// RPN FEATURE
extern vector[512][3][3][1024] rpn_conv_3x3_weight;
extern vector[512] rpn_conv_3x3_bias;
extern vector[1][38][61][512] rpn_conv_3x3;
EOPConvolution(rpn_conv_3x3, rpn_conv_3x3_weight, stage3_unit23_plus, rpn_conv_3x3_bias, 1, 1, 1, 1);

// RPN Classification: conv, reshape, softmax, reshape
extern vector[24][1][1][512] rpn_cls_score_weight;
extern vector[24] rpn_cls_score_bias;
extern vector[1][38][61][24] rpn_cls_score;
EOPConvolution(rpn_cls_score, rpn_cls_score_weight, rpn_conv_3x3, rpn_cls_score_bias, 1, 1, 0, 0);
// Reshape. No need.
// Softmax.
EOPSoftmax(rpn_cls_score);
// Reshape. No need.

// RPN BBOX
extern vector[512][1][1][48] rpn_bbox_pred_weight;
extern vector[48] rpn_bbox_pred_bias;
extern vector[1][38][61][48] rpn_bbox_pred;
EOPConvolution(rpn_bbox_pred, rpn_bbox_pred_weight, rpn_conv_3x3, rpn_bbox_pred_bias, 1, 1, 0, 0);

// *******************
// Input 1: rpn_conv_3x3
// Input 2: rpn_bbox_pred
extern vector[300][5] roi;
extern vector[1][3] im_info;
EOPProposal(rpn_conv_3x3, rpn_bbox_pred, im_info, roi);

// Input1: stage3_unit23_plus_output
// Input2: roi
extern vector[300][14][14][1024] roi_pool;
EOPRoipooling(stage3_unit23_plus, roi, roi_pool);
// *******************


// stage4_unit1
extern vector[512][1][1][1024] stage4_unit1_conv1_weight;
extern vector[300][14][14][512] stage4_unit1_conv1;
EOPConvolution(stage4_unit1_conv1, stage4_unit1_conv1_weight, roi_pool, null, 1, 1, 0, 0);
extern vector[512][1][1][512] stage4_unit1_conv2_weight;
extern vector[300][7][7][512] stage4_unit1_conv2;
EOPConvolution(stage4_unit1_conv2, stage4_unit1_conv2_weight, stage4_unit1_conv1, null, 2, 2, 1, 1);
extern vector[2048][1][1][512] stage4_unit1_conv3_weight;
extern vector[300][7][7][2048] stage4_unit1_conv3;
EOPConvolution(stage4_unit1_conv3, stage4_unit1_conv3_weight, stage4_unit1_conv2, null, 1, 1, 0, 0);
extern vector[2048][1][1][1024] stage4_unit1_sc_weight;
extern vector[300][7][7][2048] stage4_unit1_sc;
EOPConvolution(stage4_unit1_sc, stage4_unit1_sc_weight, roi_pool, null, 2, 2, 0, 0);
// EOPElementWiseADD: stage4_unit1_conv3 + stage4_unit1_sc = stage4_unit1_plus;
extern vector[300][7][7][2048] stage4_unit1_plus;
ElementWiseADD(stage4_unit1_conv3, stage4_unit1_sc, stage4_unit1_plus);

// stage4_unit2
extern vector[512][1][1][2048] stage4_unit2_conv1_weight;
extern vector[300][7][7][512] stage4_unit2_conv1;
EOPConvolution(stage4_unit2_conv1, stage4_unit2_conv1_weight, stage4_unit1_plus, null, 1, 1, 0, 0);
extern vector[512][1][1][512] stage4_unit2_conv2_weight;
extern vector[300][7][7][512] stage4_unit2_conv2;
EOPConvolution(stage4_unit2_conv2, stage4_unit2_conv2_weight, stage4_unit2_conv1, null, 1, 1, 1, 1);
extern vector[2048][1][1][512] stage4_unit2_conv3_weight;
extern vector[300][7][7][2048] stage4_unit2_conv3;
EOPConvolution(stage4_unit2_conv3, stage4_unit2_conv3_weight, stage4_unit2_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage4_unit2_conv3 + stage4_unit1_plus = stage4_unit2_plus;
extern vector[300][7][7][2048] stage4_unit2_plus;
ElementWiseADD(stage4_unit2_conv3, stage4_unit1_plus, stage4_unit2_plus);

// stage4_unit3
extern vector[512][1][1][2048] stage4_unit3_conv1_weight;
extern vector[300][7][7][512] stage4_unit3_conv1;
EOPConvolution(stage4_unit3_conv1, stage4_unit3_conv1_weight, stage4_unit2_plus, null, 1, 1, 0, 0);
extern vector[512][1][1][512] stage4_unit3_conv2_weight;
extern vector[300][7][7][512] stage4_unit3_conv2;
EOPConvolution(stage4_unit3_conv2, stage4_unit3_conv2_weight, stage4_unit3_conv1, null, 1, 1, 1, 1);
extern vector[2048][1][1][512] stage4_unit3_conv3_weight;
extern vector[300][7][7][2048] stage4_unit3_conv3;
EOPConvolution(stage4_unit3_conv3, stage4_unit3_conv3_weight, stage4_unit3_conv2, null, 1, 1, 0, 0);
// EOPElementWiseADD: stage4_unit3_conv3 + stage4_unit2_plus = stage4_unit3_plus;
extern vector[300][7][7][2048] stage4_unit3_plus;
ElementWiseADD(stage4_unit3_conv3, stage4_unit2_plus, stage4_unit3_plus);
// Pool1
extern vector[300][2048][1][1] pool1;
EOPPooling(pool1, stage4_unit3_plus, 7, 7, 7, 7, 0, 0);

// FC, rcnn classification
extern vector[2048][21] cls_score_weight;
extern vector[21] cls_score_bias;
extern vector[300][21] cls_score;
EOPFullyConnected(cls_score, cls_score_weight, pool1, cls_score_bias);

// FC, rcnn bbox classification
extern vector[2048][21] bbox_pred_weight;
extern vector[21] bbox_pred_bias;
extern vector[300][21] bbox_pred;
EOPFullyConnected(bbox_pred, bbox_pred_weight, pool1, bbox_pred_bias);
