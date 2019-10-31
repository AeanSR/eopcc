/*
    EOPSSD_MOBILENET_V1: program of SSD_MOBILENET_V1 in EOPC language.

    zengxi <zengxi@mails.ucas.ac.cn>
    2019, Oct. 8
*/

int rand;

def get_rand(x, low, high) {
  rand = rand * 6364136223846793005ULL + 1442695040888963407ULL; 
  x = ((float)((rand >> 1) & 0xFFFFFFFF) / 0x100000000) * (high - low) + low;
}
// --------------------------conv begins--------------------------------------------

// API: EOPConvolution(extern vector[NHWC/HWC] dest, extern vector[NHWC] kernel, extern vector[NHWC/HWC] input, extern vector[C] bias, 
//                                                               const int stride_x, const int stride_y, const int pad_x, const int pad_y);

// model config
// conv0
extern vector[300][300][3] image;
extern vector[32][3][3][3] weight_conv0;
extern vector[150][150][32] conv0;
EOPConvolution(conv0, weight_conv0, image, null, 2, 2, 1, 1);

// conv1_dw
extern vector[32][3][3][32] weight_conv1_dw;
extern vector[150][150][32] conv1_dw;
EOPDepthwiseConv(conv1_dw, weight_conv1_dw, conv0, null, 1, 1, 1, 1);

// conv1
extern vector[64][1][1][32] weight_conv1;
extern vector[150][150][64] conv1;
EOPConvolution(conv1, weight_conv1, conv1_dw, null, 1, 1, 0, 0);

// conv2_dw
extern vector[64][3][3][64] weight_conv2_dw;
extern vector[75][75][64] conv2_dw;
EOPDepthwiseConv(conv2_dw, weight_conv2_dw, conv1, null, 2, 2, 1, 1);

// conv2
extern vector[128][1][1][64] weight_conv2;
extern vector[75][75][128] conv2;
EOPConvolution(conv2, weight_conv2, conv2_dw, null, 1, 1, 0, 0);

// conv3_dw
extern vector[128][3][3][128] weight_conv3_dw;
extern vector[75][75][128] conv3_dw;
EOPDepthwiseConv(conv3_dw, weight_conv3_dw, conv2, null, 1, 1, 1, 1);

// conv3
extern vector[128][1][1][128] weight_conv3;
extern vector[75][75][128] conv3;
EOPConvolution(conv3, weight_conv3, conv3_dw, null, 1, 1, 0, 0);

// conv4_dw
extern vector[128][3][3][128] weight_conv4_dw;
extern vector[38][38][128] conv4_dw;
EOPDepthwiseConv(conv4_dw, weight_conv4_dw, conv3, null, 2, 2, 1, 1);

// conv4
extern vector[256][1][1][128] weight_conv4;
extern vector[38][38][256] conv4;
EOPConvolution(conv4, weight_conv4, conv4_dw, null, 1, 1, 0, 0);

// conv5_dw
extern vector[256][3][3][256] weight_conv5_dw;
extern vector[38][38][256] conv5_dw;
EOPDepthwiseConv(conv5_dw, weight_conv5_dw, conv4, null, 1, 1, 1, 1);

// conv5
extern vector[256][1][1][256] weight_conv5;
extern vector[38][38][256] conv5;
EOPConvolution(conv5, weight_conv5, conv5_dw, null, 1, 1, 0, 0);

// conv6_dw
extern vector[256][3][3][256] weight_conv6_dw;
extern vector[19][19][256] conv6_dw;
EOPDepthwiseConv(conv6_dw, weight_conv6_dw, conv5, null, 2, 2, 1, 1);

// conv6
extern vector[512][1][1][256] weight_conv6;
extern vector[19][19][512] conv6;
EOPConvolution(conv6, weight_conv6, conv6_dw, null, 1, 1, 0, 0);

// conv7_dw
extern vector[512][3][3][512] weight_conv7_dw;
extern vector[19][19][512] conv7_dw;
EOPDepthwiseConv(conv7_dw, weight_conv7_dw, conv6, null, 1, 1, 1, 1);

// conv7
extern vector[512][1][1][512] weight_conv7;
extern vector[19][19][512] conv7;
EOPConvolution(conv7, weight_conv7, conv7_dw, null, 1, 1, 0, 0);

// conv8_dw
extern vector[512][3][3][512] weight_conv8_dw;
extern vector[19][19][512] conv8_dw;
EOPDepthwiseConv(conv8_dw, weight_conv8_dw, conv7, null, 1, 1, 1, 1);

// conv8
extern vector[512][1][1][512] weight_conv8;
extern vector[19][19][512] conv8;
EOPConvolution(conv8, weight_conv8, conv8_dw, null, 1, 1, 0, 0);

// conv9_dw
extern vector[512][3][3][512] weight_conv9_dw;
extern vector[19][19][512] conv9_dw;
EOPDepthwiseConv(conv9_dw, weight_conv9_dw, conv8, null, 1, 1, 1, 1);

// conv9
extern vector[512][1][1][512] weight_conv9;
extern vector[19][19][512] conv9;
EOPConvolution(conv9, weight_conv9, conv9_dw, null, 1, 1, 0, 0);

// conv10_dw
extern vector[512][3][3][512] weight_conv10_dw;
extern vector[19][19][512] conv10_dw;
EOPDepthwiseConv(conv10_dw, weight_conv10_dw, conv9, null, 1, 1, 1, 1);

// conv10
extern vector[512][1][1][512] weight_conv10;
extern vector[19][19][512] conv10;
EOPConvolution(conv10, weight_conv10, conv10_dw, null, 1, 1, 0, 0);

// conv11_dw
extern vector[512][3][3][512] weight_conv11_dw;
extern vector[19][19][512] conv11_dw;
EOPDepthwiseConv(conv11_dw, weight_conv11_dw, conv10, null, 1, 1, 1, 1);

// conv11
extern vector[512][1][1][512] weight_conv11;
extern vector[19][19][512] conv11;
EOPConvolution(conv11, weight_conv11, conv11_dw, null, 1, 1, 0, 0);

// conv12_dw
extern vector[512][3][3][512] weight_conv12_dw;
extern vector[10][10][512] conv12_dw;
EOPDepthwiseConv(conv12_dw, weight_conv12_dw, conv11, null, 2, 2, 1, 1);

// conv12
extern vector[1024][1][1][512] weight_conv12;
extern vector[10][10][1024] conv12;
EOPConvolution(conv12, weight_conv12, conv12_dw, null, 1, 1, 0, 0);

// conv13_dw
extern vector[1024][3][3][1024] weight_conv13_dw;
extern vector[10][10][1024] conv13_dw;
EOPDepthwiseConv(conv13_dw, weight_conv13_dw, conv12, null, 1, 1, 1, 1);

// conv13
extern vector[1024][1][1][1024] weight_conv13;
extern vector[10][10][1024] conv13;
EOPConvolution(conv13, weight_conv13, conv13_dw, null, 1, 1, 0, 0);

// mobile_net ended, ssd begin

// conv14_1
extern vector[256][1][1][1024] weight_conv14_1;
extern vector[10][10][256] conv14_1;
EOPConvolution(conv14_1, weight_conv14_1, conv13, null, 1, 1, 0, 0);

// conv14_2
extern vector[512][3][3][256] weight_conv14_2;
extern vector[5][5][512] conv14_2;
EOPConvolution(conv14_2, weight_conv14_2, conv14_1, null, 2, 2, 1, 1);

// conv15_1
extern vector[128][1][1][512] weight_conv15_1;
extern vector[5][5][128] conv15_1;
EOPConvolution(conv15_1, weight_conv15_1, conv14_2, null, 1, 1, 0, 0);

// conv15_2
extern vector[256][3][3][128] weight_conv15_2;
extern vector[3][3][256] conv15_2;
EOPConvolution(conv15_2, weight_conv15_2, conv15_1, null, 2, 2, 1, 1);

// conv16_1
extern vector[128][1][1][256] weight_conv16_1;
extern vector[3][3][128] conv16_1;
EOPConvolution(conv16_1, weight_conv16_1, conv15_2, null, 1, 1, 0, 0);

// conv16_2
extern vector[256][3][3][128] weight_conv16_2;
extern vector[2][2][256] conv16_2;
EOPConvolution(conv16_2, weight_conv16_2, conv16_1, null, 2, 2, 1, 1);

// conv17_1
extern vector[64][1][1][256] weight_conv17_1;
extern vector[2][2][64] conv17_1;
EOPConvolution(conv17_1, weight_conv17_1, conv16_2, null, 1, 1, 0, 0);

// conv17_2
extern vector[128][3][3][64] weight_conv17_2;
extern vector[1][1][128] conv17_2;
EOPConvolution(conv17_2, weight_conv17_2, conv17_1, null, 2, 2, 1, 1);

// conv11_mbox_loc
extern vector[12][1][1][512] weight_conv11_mbox_loc;
extern vector[19][19][12] conv11_mbox_loc;
EOPConvolution(conv11_mbox_loc, weight_conv11_mbox_loc, conv11, null, 1, 1, 0, 0);

// conv11_mbox_conf
extern vector[63][1][1][512] weight_conv11_mbox_conf;
extern vector[19][19][63] conv11_mbox_conf;
EOPConvolution(conv11_mbox_conf, weight_conv11_mbox_conf, conv11, null, 1, 1, 0, 0);

// conv13_mbox_loc
extern vector[24][1][1][1024] weight_conv13_mbox_loc;
extern vector[10][10][24] conv13_mbox_loc;
EOPConvolution(conv13_mbox_loc, weight_conv13_mbox_loc, conv13, null, 1, 1, 0, 0);

// conv13_mbox_conf
extern vector[126][1][1][1024] weight_conv13_mbox_conf;
extern vector[10][10][126] conv13_mbox_conf;
EOPConvolution(conv13_mbox_conf, weight_conv13_mbox_conf, conv13, null, 1, 1, 0, 0);

// conv14_2_mbox_loc
extern vector[24][1][1][512] weight_conv14_2_mbox_loc;
extern vector[5][5][24] conv14_2_mbox_loc;
EOPConvolution(conv14_2_mbox_loc, weight_conv14_2_mbox_loc, conv14_2, null, 1, 1, 0, 0);

// conv14_2_mbox_conf
extern vector[126][1][1][512] weight_conv14_2_mbox_conf;
extern vector[5][5][126] conv14_2_mbox_conf;
EOPConvolution(conv14_2_mbox_conf, weight_conv14_2_mbox_conf, conv14_2, null, 1, 1, 0, 0);

// conv15_2_mbox_loc
extern vector[24][1][1][256] weight_conv15_2_mbox_loc;
extern vector[3][3][24] conv15_2_mbox_loc;
EOPConvolution(conv15_2_mbox_loc, weight_conv15_2_mbox_loc, conv15_2, null, 1, 1, 0, 0);

// conv15_2_mbox_conf
extern vector[126][1][1][256] weight_conv15_2_mbox_conf;
extern vector[3][3][126] conv15_2_mbox_conf;
EOPConvolution(conv15_2_mbox_conf, weight_conv15_2_mbox_conf, conv15_2, null, 1, 1, 0, 0);

// conv16_2_mbox_loc
extern vector[24][1][1][256] weight_conv16_2_mbox_loc;
extern vector[2][2][24] conv16_2_mbox_loc;
EOPConvolution(conv16_2_mbox_loc, weight_conv16_2_mbox_loc, conv16_2, null, 1, 1, 0, 0);

// conv16_2_mbox_conf
extern vector[126][1][1][256] weight_conv16_2_mbox_conf;
extern vector[2][2][126] conv16_2_mbox_conf;
EOPConvolution(conv16_2_mbox_conf, weight_conv16_2_mbox_conf, conv16_2, null, 1, 1, 0, 0);

// conv17_2_mbox_loc
extern vector[24][1][1][128] weight_conv17_2_mbox_loc;
extern vector[1][1][24] conv17_2_mbox_loc;
EOPConvolution(conv17_2_mbox_loc, weight_conv17_2_mbox_loc, conv17_2, null, 1, 1, 0, 0);

// conv17_2_mbox_conf
extern vector[126][1][1][128] weight_conv17_2_mbox_conf;
extern vector[1][1][126] conv17_2_mbox_conf;
EOPConvolution(conv17_2_mbox_conf, weight_conv17_2_mbox_conf, conv17_2, null, 1, 1, 0, 0);

print "conv end";
// --------------------------conv end--------------------------------------------

// ---------------------------util functions------------------------------------
const float img_height = 300; 
const float img_width = 300;

float global_variance[4]; //  = (0.1, 0.1, 0.2, 0.2);
global_variance[0] = 0.1; global_variance[1] = 0.1; global_variance[2] = 0.2; global_variance[3] = 0.2;
const float offset = 0.5;
int step_h = 0;
int step_w = 0;

def sqrt(num, ans) {
  float a = 0.0;
  float b = num + 0.25;
  float m;
  while (1) {
    m = (b + a) / 2;
    if ((m - a) < 0.00005) break;
    if ((b - m) < 0.00005) break;
    if ((m * m - num) * (a * a - num) < 0) b = m;
    else a = m;
  } // end while
  ans = m;
}

def PriorBox(layer_width, layer_height, top_data, begin_idx, min_size, max_size, aspect_ratio, ratio_size) {
  step_w = img_width / layer_width;
  step_h = img_height / layer_height;
 
  int idx = begin_idx; 
  int num_priors = ratio_size;
  int h;
  int w;
  for (h = 0; h < layer_height; ++h) {
    for (w = 0; w < layer_width; ++w) {
      float center_x = (w + offset) * step_w;
      float center_y = (h + offset) * step_h;
      float box_width; 
      float box_height;
      box_width = min_size;
      box_height = min_size;

      top_data[0][idx++] = (center_x - box_width / 2.) / img_width;
      top_data[0][idx++] = (center_y - box_height / 2.) / img_height;
      top_data[0][idx++] = (center_x + box_width / 2.) / img_width;
      top_data[0][idx++] = (center_y + box_height / 2.) / img_height;  
      
      if (max_size != 0) {
        num_priors = ratio_size + 1;
        sqrt(min_size * max_size, box_width);
        box_height = box_width;
        top_data[0][idx++] = (center_x - box_width / 2.) / img_width;
        top_data[0][idx++] = (center_y - box_height / 2.) / img_height;
        top_data[0][idx++] = (center_x + box_width / 2.) / img_width;
        top_data[0][idx++] = (center_y + box_height / 2.) / img_height;
      }
      int r;
      for (r = 0; r < ratio_size; ++r) {
        float ar = aspect_ratio[r];
        if ((ar-1) * (ar - 1) > 0.00005) {  // check ar != 1
          float temp;
          sqrt(ar, temp);
          box_width = min_size * temp;
          box_height = min_size / temp;
          top_data[0][idx++] = (center_x - box_width / 2.) / img_width;
          top_data[0][idx++] = (center_y - box_height / 2.) / img_height;
          top_data[0][idx++] = (center_x + box_width / 2.) / img_width;
          top_data[0][idx++] = (center_y + box_height / 2.) / img_height;
        } 
      }
    }                                                               
  } // end for h

  int h;
  int w;
  int i;
  int j; 
  idx = begin_idx;
  for (h = 0; h < layer_height; ++h) {
    for (w = 0; w < layer_width; ++w) {
      for (i = 0; i < num_priors; ++i) {
        for (j = 0; j < 4; ++j) {
          top_data[1][idx++] = global_variance[j];
        }
      }
    }
  }
}

// ---------------------------util functions end------------------------------------


//------------------------------------- concat---------------------------------------

float mbox_priorbox[2][7668];
// output shape : layer_width * lyaer_height * num_priors * 4
// num_priors = aspect_ratios.length * min_size.length + max_size.length

// compute conv11_mbox_priorbox
// extern vector[19][19][512] conv11;
// for conv11, max_size.length = 0, min_size = 1, aspect_ratio.size = 3
int feature_width = 19;
int feature_height = 19;
float min_size = 60.0;
float max_size = 0.0;
//vector[3] conv11_aspect_ratio; //  = (1, 1/2.0, 2.0);
float conv11_aspect_ratio[3];
conv11_aspect_ratio[0] = 1.; conv11_aspect_ratio[1] = 0.5; conv11_aspect_ratio[2] = 2.;
// vector[2][19*19*3*4] conv11_mbox_priorbox;
except h1 = async PriorBox(feature_width, feature_height, mbox_priorbox, 0, min_size, max_size, conv11_aspect_ratio, 3);
// PriorBox(feature_width, feature_height, mbox_priorbox, 0, min_size, max_size, conv11_aspect_ratio, 3);

// compute conv13_mbox_priorbox
// extern vector[10][10][1024] conv13;
// for conv13, max_size.length = 1, min_size = 1, aspect_ratio.size = 5
int feature_width2 = 10;
int feature_height2 = 10;
min_size = 105.0;
max_size = 150.0;
//vector[5] global_aspect_ratio; //  = (1, 1/2.0, 2.0, 1/3.0, 3.0);
float global_aspect_ratio[5];
global_aspect_ratio[0] = 1.; global_aspect_ratio[1] = 0.5; global_aspect_ratio[2] = 2.; global_aspect_ratio[3] = 1. / 3; global_aspect_ratio[4] = 3.; 
// vector[2][10*10*6*4] conv13_mbox_PriorBox;
except h2 = async PriorBox(feature_width2, feature_height2, mbox_priorbox, 4332, min_size, max_size, global_aspect_ratio, 5);
// PriorBox(feature_width, feature_height, mbox_priorbox, 4332, min_size, max_size, global_aspect_ratio, 5);

// compute conv14_2_mbox_PriorBox
// extern vector[5][5][512] conv14_2;
// for conv14_2, max_size.length = 1, min_size = 1, aspect_ratio.size = 5
int feature_width3 = 5;
int feature_height3 = 5;
min_size = 150.0;
max_size = 195.0;
// vector[2][5*5*6*4] conv14_2_mbox_PriorBox;
except h3 = async PriorBox(feature_width3, feature_height3, mbox_priorbox, 6732, min_size, max_size, global_aspect_ratio, 5);
// PriorBox(feature_width, feature_height, mbox_priorbox, 6732, min_size, max_size, global_aspect_ratio, 5);

// compute conv15_2_mbox_PriorBox
// extern vector[3][3][256] conv15_2;
// for conv15_2, max_size.length = 1, min_size = 1, aspect_ratio.size = 5
int feature_width4 = 3;
int feature_height4 = 3;
min_size = 195.0;
max_size = 240.0;
// vector[2][3*3*6*4] conv15_2_mbox_PriorBox;
except h4 = async PriorBox(feature_width4, feature_height4, mbox_priorbox, 7332, min_size, max_size, global_aspect_ratio, 5);
// PriorBox(feature_width, feature_height, mbox_priorbox, 7332, min_size, max_size, global_aspect_ratio, 5);

// compute conv16_2_mbox_PriorBox
// extern vector[2][2][256] conv16_2;
// for conv16_2, max_size.length = 1, min_size = 1, aspect_ratio.size = 5
int feature_width5 = 2;
int feature_height5 = 2;
min_size = 240.0;
max_size = 285.0;
// vector[2][2*2*6*4] conv16_2_mbox_PriorBox;
except h5 = async PriorBox(feature_width5, feature_height5, mbox_priorbox, 7548, min_size, max_size, global_aspect_ratio, 5);
// PriorBox(feature_width, feature_height, mbox_priorbox, 7548, min_size, max_size, global_aspect_ratio, 5);

// compute conv17_2_mbox_PriorBox
// extern vector[1][1][128] conv17_2;
// for conv17_2, max_size.length = 1, min_size = 1, aspect_ratio.size = 5
int feature_width6 = 1;
int feature_height6 = 1;
min_size = 285.0;
max_size = 300.0;
// vector[2][1*1*6*4] conv17_2_mbox_PriorBox;
except h6 = async PriorBox(feature_width6, feature_height6, mbox_priorbox, 7664, min_size, max_size, global_aspect_ratio, 5);
// PriorBox(feature_width, feature_height, mbox_priorbox, 7664, min_size, max_size, global_aspect_ratio, 5);

await h1;
await h2;
await h3;
await h4;
await h5;
await h6;



//------------------------------------- concat end---------------------------------------

def exp(x) {
  x = 1.0 + x/256;
  x *= x;  
  x *= x;  
  x *= x;  
  x *= x;  
  x *= x;  
  x *= x;  
  x *= x;  
  x *= x;  
} 

def swap(arr, left, right, cols) {
  int i;
  for (i = 0; i < cols; ++i) {
    int temp = arr[left][i];
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
  p = i + 1;  // return p
}

def sort_score(pairs, rows, cols) {
  // quick sort
  int l = 0;
  int h = rows - 1;
  int stack[2100];
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
    if (p + 1 < h) {
      stack[++top] = p + 1;
      stack[++top] = h;
    }
    //count += 1;
    //if (count == 15000) break;
  }
   
}


vector[1917][2] score_pairs;
def getMaxScoreIndex(scores, outputs) {
  int i = 0;
  for (i = 0; i < 1917; ++i) {
    score_pairs[i][0] = scores[i];
    score_pairs[i][1] = (float)i;
  }
  sort_score(score_pairs, 1917, 2);
  for (i = 0; i < 100; ++i) { // get top_k scores' idx
    outputs[i] = score_pairs[i][1];
  }
}

def BBoxSize(bbox, size) {
  float width = bbox[2] - bbox[0];
  float height = bbox[3] - bbox[1];
  size = (width + 1) * (height + 1);
}

def JaccardOverlap(bbox1, bbox2, overlap) {
  float intersection[4];
  intersection[0] = bbox2[0] -= bbox1[0]; get_rand(intersection[0], -1, 1);
  intersection[1] = bbox2[1] -= bbox1[1]; get_rand(intersection[1], -1, 1);
  intersection[2] = bbox2[2] -= bbox1[2]; get_rand(intersection[2], -1, 1);
  intersection[3] = bbox2[3] -= bbox1[3]; get_rand(intersection[3], -1, 1);
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
    float a = bbox1[0]; get_rand(a, 0, 1);
    float b = bbox2[0]; get_rand(a, 0, 1);
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

vector[40257] conf_data;// 1917 * 21
vector[7668]  loc_data;  // 1917 * 4
vector[2][7668] mbox_priorbox;

// compute mbox_conf
// concat conv11_mbox_conf, conv13_mbox_conf, conv14_2_mbox_conf
// conv15_2_mbox_conf, conv16_2_mbox_conf

// 22743 + 12600 + 3150 + 1134 + 504 + 126 = 40257
// extern vector[19][19][63] conv11_mbox_conf; 
// extern vector[10][10][126] conv13_mbox_conf;
// extern vector[5][5][126] conv14_2_mbox_conf;
// extern vector[3][3][126] conv15_2_mbox_conf;
// extern vector[2][2][126] conv16_2_mbox_conf;
// extern vector[1][1][126] conv17_2_mbox_conf;

// concat conf_data, load data to SPM
(vector[22743])conf_data[0] = (extern vector[22743])conv11_mbox_conf;
(vector[12600])conf_data[22743] = (extern vector[12600])conv13_mbox_conf;
(vector[3150])conf_data[35343] = (extern vector[3150])conv14_2_mbox_conf;
(vector[1134])conf_data[38493] = (extern vector[1134])conv15_2_mbox_conf;
(vector[504])conf_data[39627] = (extern vector[504])conv16_2_mbox_conf;
(vector[126])conf_data[40131] = (extern vector[126])conv17_2_mbox_conf;

// transpose conf_data [1917 * 21]  to [21 * 1917]
vector[40257] conf_data_reshape;
int i = 0;
int j = 0;
for (i = 0; i < 1917; ++i) {
  for (j = 0; j < 21; ++j) {
    conf_data_reshape[j*1917+i] = conf_data[i*21+j];
  }
}

// compute mbox_loc
// concat conv11_mbox_loc, conv13_mbox_loc, conv14_2_mbox_loc
// conv15_2_mbox_loc, conv16_2_mbox_loc

// 4332 + 2400 + 600 + 216 + 96 + 24 = 7668
// extern vector[19][19][12] conv11_mbox_loc;
// extern vector[10][10][24] conv13_mbox_loc;
// extern vector[5][5][24] conv14_2_mbox_loc;
// extern vector[3][3][24] conv15_2_mbox_loc;
// extern vector[2][2][24] conv16_2_mbox_loc;
// extern vector[1][1][24] conv17_2_mbox_loc;


// concat loc_data, load data to SPM
(vector[4332])loc_data[0] = (extern vector[4332])conv11_mbox_loc;
(vector[2400])loc_data[4332] = (extern vector[2400])conv13_mbox_loc;
(vector[600])loc_data[6732] = (extern vector[600])conv14_2_mbox_loc;
(vector[216])loc_data[7332] = (extern vector[216])conv15_2_mbox_loc;
(vector[96])loc_data[7548] = (extern vector[96])conv16_2_mbox_loc;
(vector[24])loc_data[7644] = (extern vector[24])conv17_2_mbox_loc;

// other parameters
int num_classes = 21;
int num_priors = 1917;
int num_loc_class = 1;
int background_label_id = 0;
float nms_threshold = 0.45;
int top_k = 100;
float confidence_threshold = 0.60;
float eta=1.0;

// decode boxes
vector[1917*4] prior_boxes;
prior_boxes = (vector[1917*4])mbox_priorbox[0];
vector[1917*4] prior_variances;
prior_variances = (vector[1917*4])mbox_priorbox[1];
vector[1917][4] decode_bboxes;
int i;
for (i = 0; i < num_priors; ++i) {
  int start = i * 4;
  // start: xmin; start+1: ymin; start+2: xmax; start+3: ymax
  float prior_width = prior_boxes[start+2] - prior_boxes[start];
  float prior_height = prior_boxes[start+3] - prior_boxes[start+1];
  float prior_center_x = (prior_boxes[start] + prior_boxes[start+2]) / 2.;
  float prior_center_y = (prior_boxes[start+1] + prior_boxes[start+3]) / 2.;
  
  float decode_bbox_center_x;
  float decode_bbox_center_y;
  float decode_bbox_width;
  float decode_bbox_height;
  decode_bbox_center_x = prior_variances[start] * loc_data[start] * prior_width + prior_center_x;
  decode_bbox_center_y = prior_variances[start+1] * loc_data[start+1] * prior_height + prior_center_y;
  float temp;
  temp = prior_variances[start+2] * loc_data[start+2];
  exp(temp);
  decode_bbox_width = temp * prior_width;
  temp = prior_variances[start+3] * loc_data[start+3];
  exp(temp);
  decode_bbox_height = temp * prior_height;

  decode_bboxes[i][0] = decode_bbox_center_x - decode_bbox_width / 2.;
  decode_bboxes[i][1] = decode_bbox_center_y - decode_bbox_height / 2.;
  decode_bboxes[i][2] = decode_bbox_center_x + decode_bbox_width / 2.;
  decode_bboxes[i][3] = decode_bbox_center_y + decode_bbox_height / 2.;
}

int total_num_det = 0;
int indices[21][100];// indices; // max valid scores of one class is 100
int num_dets[21]; // how many valid scores of every class
//vector[21] num_dets;
for (i = 0; i < num_classes; ++i) {
  // apply nms
  vector[100] score_index_vec;  // top_k = 100
  getMaxScoreIndex((vector[1917])conf_data_reshape[i*1917], score_index_vec);
  print("get max score end");
  
  int idx = 0;
  int k = 0;
  int indices_count = 0;
  for (idx = 0; idx < 100; ++idx) {
    int keep = 1;
    for (k = 0; k < indices_count; ++k) {
      if (keep == 1) {
        int bbox_idx = score_index_vec[idx];
        int former_idx = indices[i][k];
        float overlap;
        JaccardOverlap(decode_bboxes[bbox_idx], decode_bboxes[former_idx], overlap);
        if (overlap > nms_threshold) keep = 0;
      } else {
        break;
      }
    }
    if (keep) {
      indices[i][indices_count] = idx;
      indices_count += 1;
    }
  }
  num_dets[i] = indices_count;
  total_num_det += indices_count;
}

int idx = 0;
vector[100][3] score_index_pairs;
for (i = 0; i < 21; ++i) {
  int hacked;
  get_rand(hacked, 0, 100);
  print "score index pairs ", i, " loops for ", hacked;
  for (j = 0; j < (int)num_dets[i]; ++j) {
    int score_idx = indices[i][j];
    score_index_pairs[idx][0] = conf_data_reshape[i*21 + score_idx];
    score_index_pairs[idx][1] = i;  // label class
    score_index_pairs[idx++][2] = score_idx;
  }
}
print("start sort");
sort_score(score_index_pairs, total_num_det, 3);
print("end sort");

if (total_num_det > 100) total_num_det = 100;
// copy data to output vector
float detection_out[100][7];// detection_out; //  [image_id, label, confidence, xmin, ymin, xmax, ymax]
for (i = 0; i < total_num_det; ++i) {
  idx = score_index_pairs[i][2];
  detection_out[i][0] = 0; // index
  detection_out[i][1] = score_index_pairs[i][1]; // index
  detection_out[i][2] = score_index_pairs[i][0]; // index
  detection_out[i][3] = decode_bboxes[idx][0]; // index
  detection_out[i][4] = decode_bboxes[idx][1]; // index
  detection_out[i][5] = decode_bboxes[idx][2]; // index
  detection_out[i][6] = decode_bboxes[idx][3]; // index
}

// write output to extern vector, finished!
//extern vector[100][7] detection_out_extern;
//(extern vector[100][7])detection_out_extern = detection_out;
