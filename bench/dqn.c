/*
    EOPDQN: sample program of DQN-MsPacman in EOPC language.

    zhaoyongwei <zhaoyongwei@ict.ac.cn>
    2020, Apr. 6
*/

const int input_h = 88; const int input_w = 80;
const int ch_1 = 32; const int h1 = 81; const int h1p = 20; const int w1 = 73; const int w1p = 18;
const int ch_2 = 64; const int h2 = 17; const int h2p = 8; const int w2 = 15; const int w2p = 7;
const int ch_3 = 64; const int h3 = 6; const int h3p = 3; const int w3 = 5; const int w3p = 2;
const int ch_flat = 384;
const int ch_4 = 512;

int rand = 14615;

def get_rand(x, low, high) {
  rand = rand * 6364136223846793005ULL + 1442695040888963407ULL; 
  x = ((float)((rand >> 1) & 0xFFFFFFFF) / 0x100000000) * (high - low) + low;
}

const float GAMMA = 0.95;
const float LEARNING_RATE = 0.001;
const int MEMORY_SIZE = 100000;
const int BATCH_SIZE = 20;
const float EXPLORATION_MAX = 1.0;
const float EXPLORATION_MIN = 0.01;
const float EXPLORATION_DECAY = 0.995;

float exploration_rate = EXPLORATION_MAX;
const int action_space = 5;
const int observation_space = input_h * input_w;
extern vector[MEMORY_SIZE][observation_space] memory_state;
extern vector[MEMORY_SIZE] memory_action;
extern vector[MEMORY_SIZE] memory_reward;
extern vector[MEMORY_SIZE][observation_space] memory_next_state;
extern vector[MEMORY_SIZE] memory_done;
int memory_pointer = 0;

def remember(state, action, reward, next_state, done) {
  memory_state[memory_pointer] = state;
  memory_action[memory_pointer] = action;
  memory_reward[memory_pointer] = reward;
  memory_next_state[memory_pointer] = next_state;
  memory_done[memory_pointer] = done;
  memory_pointer++;
}

extern vector[ch_1][8][8][1] wt_l1_ex;
extern vector[ch_2][4][4][ch_1] wt_l2_ex;
extern vector[ch_3][3][3][ch_2] wt_l3_ex;
extern vector[ch_4][ch_flat] wt_l4_ex;
extern vector[action_space][ch_4] wt_l5_ex;


def predict(state, q_values) {
  intern vector[h1p][w1p][ch_1] d1p;
  intern decltype(wt_l2_ex) wt2;
  intern decltype(wt_l1_ex) wt1;
  {
    vector[h1][w1][ch_1] d1;
    wt1 = wt_l1_ex; wt2 = wt_l2_ex;
    conv d1, wt1, (vector[input_h][input_w][1])state;
    pool d1p, d1, 4, 4, 4, 4;
    act d1p, d1p;
  }
  intern decltype(wt_l3_ex) wt1;
  intern vector[h2p][w2p][ch_2] d2p;
  {
    vector[h2][w2][ch_2] d2;
    wt1 = wt_l3_ex;
    conv d2, wt2, d1p;
    pool d2p, d2, 2, 2, 2, 2;
    act d2p, d2p;
  }
  intern decltype(wt_l4_ex) wt2;
  intern vector[h3p][w3p][ch_3] d1p;
  {
    vector[h3][w3][ch_3] d1;
    wt2 = wt_l4_ex;
    conv d1, wt1, d2p;
    pool d1p, d1, 2, 2, 2, 2;
    act d1p, d1p;
  }
  intern decltype(wt_l5_ex) wt1;
  intern vector[1][ch_4] d2p;
  {
    wt1 = wt_l5_ex;
    mm d2p, wt2, (vector[1][ch_flat])d1p;
    act d2p, d2p;
  }
  int wt2;
  {
    mm (vector[1][action_space])q_values, wt1, d2p;
    act q_values, q_values;
  }
}

def act_(state, action) {
  float r;
  get_rand(r, 0, 1);
  if (r < exploration_rate) {
    get_rand(r, 0, action_space);
    action = (int)r;
  } else {
    vector[action_space] q_values;
    predict(state, q_values);
    vector max = |q_values;
    q_values = q_values < max;
    int i;
    for (i = 0; i < action_space; i++)
      if ((q_values[i], 0) == 0) action = i;
  }
}

def fit(state, q_values) {
  intern vector[h1p][w1p][ch_1] d1p;
  intern decltype(wt_l2_ex) wt2;
  intern decltype(wt_l1_ex) wt1;
  extern vector[h1p][w1p][ch_1] exd1;
  {
    vector[h1][w1][ch_1] d1;
    wt1 = wt_l1_ex; wt2 = wt_l2_ex;
    conv d1, wt1, (vector[input_h][input_w][1])state;
    pool d1p, d1, 4, 4, 4, 4;
    act d1p, d1p;
    exd1 = d1p;
  }
  intern decltype(wt_l3_ex) wt1;
  intern vector[h2p][w2p][ch_2] d2p;
  extern vector[h2p][w2p][ch_2] exd2;
  {
    vector[h2][w2][ch_2] d2;
    wt1 = wt_l3_ex;
    conv d2, wt2, d1p;
    pool d2p, d2, 2, 2, 2, 2;
    act d2p, d2p;
    exd2 = d2p;
  }
  intern decltype(wt_l4_ex) wt2;
  intern vector[h3p][w3p][ch_3] d1p;
//  extern vector[h3p][w3p][ch_3] exd3;
  {
    vector[h3][w3][ch_3] d1;
    wt2 = wt_l4_ex;
    conv d1, wt1, d2p;
    pool d1p, d1, 2, 2, 2, 2;
    act d1p, d1p;
//    exd3 = d1p;
  }
  intern decltype(wt_l5_ex) wt1;
  intern vector[1][ch_4] d2p;
  {
    wt1 = wt_l5_ex;
    mm d2p, wt2, (vector[1][ch_flat])d1p;
    act d2p, d2p;
  }
  //int wt2; int d1p;
  intern vector[1][action_space] d5;
  {
    mm d5, wt1, d2p;
    act d5, d5;
  }
  d5 = ((decltype(d5))q_values - d5) * 2;
  {
    decltype(wt1) gradient;
    act d5, d5;
    mm gradient, d2p, (vector[action_space][1])d5, 1;
    mm d2p, wt1, d5, 1;
    gradient *= LEARNING_RATE; wt1 += gradient;
    wt_l5_ex = wt1;
  }
  int d5; int wt1;
  {
    decltype(wt2) gradient;
    act d2p, d2p;
    mm gradient, (vector[1][ch_flat])d1p, (vector[ch_4][1])d2p, 1;
    mm (vector[1][ch_flat])d1p, wt2, d2p, 1;
    gradient *= LEARNING_RATE; wt2 += gradient;
    wt_l4_ex = wt2;
  }
  int d2p; int wt2;
  intern decltype(exd2) d2p;
  {
    intern decltype(wt_l3_ex) wt3; decltype(wt3) gradient;
    d2p = exd2; wt3 = wt_l3_ex;
    act d1p, d1p;
    intern vector[h3][w3][ch_3] d1;
    unpool d1, d1p, 2, 2, 2, 2;
    reconv gradient, d1, d2p;
    deconv d2p, wt3, d1;
    gradient *= LEARNING_RATE; wt3 += gradient;
    wt_l3_ex = wt3; 
  }
  int d1p;
  intern decltype(exd1) d1p;
  {
    intern decltype(wt_l2_ex) wt2; decltype(wt2) gradient;
    d1p = exd1; wt2 = wt_l2_ex;
    act d2p, d2p;
    intern vector[h2][w2][ch_2] d2;
    unpool d2, d2p, 2, 2, 2, 2;
    reconv gradient, d2, d1p;
    deconv d1p, wt2, d2;
    gradient *= LEARNING_RATE; wt2 += gradient;
    wt_l2_ex = wt2; 
  }
  int d2p;
  {
    intern decltype(wt_l1_ex) wt1; decltype(wt1) gradient;
    wt1 = wt_l1_ex;
    act d1p, d1p;
    intern vector[h1][w1][ch_1] d1;
    unpool d1, d1p, 4, 4, 4, 4;
    reconv gradient, d1, (vector[input_h][input_w][1])state;
    gradient *= LEARNING_RATE; wt1 += gradient;
    wt_l1_ex = wt1; 
  }
}

def experience_replay() {
  if (memory_pointer < BATCH_SIZE) {} else {
  int selection_range = memory_pointer;
  int i = 0;
  for (i = 0; i < BATCH_SIZE; i++) {
    int select_idx;
    get_rand(select_idx, 0, selection_range);
    vector[observation_space] state;
    vector action;
    vector reward;
    vector[observation_space] next_state;
    vector done;
    state = memory_state[select_idx];
    action = memory_action[select_idx];
    reward = memory_reward[select_idx];
    next_state = memory_next_state[select_idx];
    done = memory_done[select_idx];

    vector q_update;
    vector[action_space] predict_next;
    vector[action_space] q_values;
    predict(next_state, predict_next);
    q_update = reward + GAMMA * |predict_next;
    predict(state, q_values);
    q_values[(action, 0)] = q_update;
    fit(state, q_values);

    def swap(l, r) {
      intern decltype(l) t1;
      intern decltype(r) t2;
      t1 = l; t2 = r;
      l = t2; r = t1;
    }
    selection_range--;
    swap(memory_state[select_idx], memory_state[selection_range]);
    swap(memory_action[select_idx], memory_action[selection_range]);
    swap(memory_reward[select_idx], memory_reward[selection_range]);
    swap(memory_next_state[select_idx], memory_next_state[selection_range]);
    swap(memory_done[select_idx], memory_done[selection_range]);
  }
    exploration_rate *= EXPLORATION_DECAY;
    exploration_rate = ((EXPLORATION_MIN > exploration_rate) ? EXPLORATION_MIN : exploration_rate);
  }
}

def random_state(state) {
  act state, state;
  //int i;
  //for (i = 0; i < observation_space; i++)
  //  get_rand(state[i], 0, 1);
}

extern vector[4][input_h][input_w][1] blend_buf;
int blend_pointer = 0;

def blend(frame, output) {
  extern vector[input_h][input_w][3] crop;
  int i;
  for (i = 0; i < input_h; i+=8) {
    strideio crop[i+0], frame[2*i+1], sizeof(crop[0][0]), sizeof(frame[0][0]), input_w;
    strideio crop[i+1], frame[2*i+3], sizeof(crop[0][0]), sizeof(frame[0][0]), input_w;
    strideio crop[i+2], frame[2*i+5], sizeof(crop[0][0]), sizeof(frame[0][0]), input_w;
    strideio crop[i+3], frame[2*i+7], sizeof(crop[0][0]), sizeof(frame[0][0]), input_w;
    strideio crop[i+4], frame[2*i+9], sizeof(crop[0][0]), sizeof(frame[0][0]), input_w;
    strideio crop[i+5], frame[2*i+11], sizeof(crop[0][0]), sizeof(frame[0][0]), input_w;
    strideio crop[i+6], frame[2*i+13], sizeof(crop[0][0]), sizeof(frame[0][0]), input_w;
    strideio crop[i+7], frame[2*i+15], sizeof(crop[0][0]), sizeof(frame[0][0]), input_w;
  }
  vector[input_h][input_w][1] greyscale;
  vector[input_h][input_w][1] tmp;
  strideio greyscale, crop, sizeof(greyscale[0][0]), sizeof(crop[0][0]), input_h * input_w;
  strideio tmp, (extern vector[0])crop[0][0][1], sizeof(greyscale[0][0]), sizeof(crop[0][0]), input_h * input_w;
  greyscale *= 1. / 3;
  tmp *= 1. / 3;
  greyscale += tmp;
  strideio tmp, (extern vector[0])crop[0][0][2], sizeof(greyscale[0][0]), sizeof(crop[0][0]), input_h * input_w;
  tmp *= 1. / 3;
  greyscale += tmp;
  tmp = (greyscale != (210 + 164 + 74) / 3);
  greyscale *= tmp;
  greyscale = (greyscale - 128) / 128 - 1;
  blend_buf[blend_pointer % 4] = greyscale;
  blend_pointer++;
  int n = blend_pointer > 4 ? 4 : blend_pointer;
  greyscale *= 0;
  for (i = 0; i < n; i++) {
    tmp = blend_buf[i];
    tmp *= 1. / n;
    greyscale += tmp;
  }
  output = (decltype(output))greyscale;
}

int run;
for (run = 0; run < 10; run++) {

  vector[observation_space] state;
  vector[observation_space] next_state;
  {  vector[178][160][3] raw_state;
     random_state(raw_state);
     blend(raw_state, state); }
  int action;
  int terminal;
  float reward;
  int step = 0;
  while (1) {
    step += 1;
    act_(state, action);
    {  vector[178][160][3] raw_state;
       random_state(raw_state);
       blend(raw_state, next_state); }
    get_rand(reward, 0, 1);
    terminal = step > 50;
    reward = terminal ? -reward : reward;
    vector terminal_vec; vector reward_vec; vector action_vec;
    terminal_vec = terminal; reward_vec = reward; action_vec = action;
    remember(state, action_vec, reward_vec, next_state, terminal_vec);
    state = next_state;
    if (terminal) {
      print "Run: ", run, ", exploration: ", exploration_rate, ", score: ", step;
      break;
    }
    experience_replay();
  }
}
