/*
    EOPDQN: sample program of DQN-Cartpole in EOPC language.

    zhaoyongwei <zhaoyongwei@ict.ac.cn>
    2020, Apr. 6
*/

const int ch_1 = 24;
const int ch_2 = 24;

int rand = 14615;

def get_rand(x, low, high) {
  rand = rand * 6364136223846793005ULL + 1442695040888963407ULL; 
  x = ((float)((rand >> 1) & 0xFFFFFFFF) / 0x100000000) * (high - low) + low;
}

const float GAMMA = 0.95;
const float LEARNING_RATE = 0.001;
const int MEMORY_SIZE = 1000000;
const int BATCH_SIZE = 10;
const float EXPLORATION_MAX = 1.0;
const float EXPLORATION_MIN = 0.01;
const float EXPLORATION_DECAY = 0.995;

float exploration_rate = EXPLORATION_MAX;
const int action_space = 1;
const int observation_space = 4;
extern vector[MEMORY_SIZE][4] memory_state;
extern vector[MEMORY_SIZE] memory_action;
extern vector[MEMORY_SIZE] memory_reward;
extern vector[MEMORY_SIZE][4] memory_next_state;
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

extern vector[ch_1][observation_space] wt_l1_ex;
extern vector[ch_2][ch_1] wt_l2_ex;
extern vector[action_space][ch_2] wt_l3_ex;

intern decltype(wt_l1_ex) wt_l1;
intern decltype(wt_l2_ex) wt_l2;
intern decltype(wt_l3_ex) wt_l3;

wt_l1 = wt_l1_ex;
wt_l2 = wt_l2_ex;
wt_l3 = wt_l3_ex;

def predict(state, q_values) {
  vector[1][ch_1] tmp1;
  vector[1][ch_2] tmp2;
  mm tmp1, wt_l1, (vector[1][observation_space])state;
  mm tmp2, wt_l2, tmp1;
  mm (vector[1][action_space])q_values, wt_l3, tmp2;
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
  vector[1][observation_space] tmp0;
  vector[1][ch_1] tmp1;
  vector[1][ch_2] tmp2;
  vector[1][action_space] tmp3;
  tmp0 = state;
  mm tmp1, wt_l1, tmp0;
  mm tmp2, wt_l2, tmp1;
  mm tmp3, wt_l3, tmp2;
  tmp3 = 2 * (tmp3[0] - q_values);
  decltype(wt_l3) gradient;
  mm gradient, tmp2, (vector[action_space][1])tmp3, 1;
  mm tmp2, wt_l3, tmp3, 1;
  gradient *= LEARNING_RATE;
  wt_l3 += gradient;
  decltype(wt_l2) gradient;
  mm gradient, tmp1, (vector[ch_2][1])tmp2, 1;
  mm tmp1, wt_l2, tmp2, 1;
  gradient *= LEARNING_RATE;
  wt_l2 += gradient;
  decltype(wt_l1) gradient;
  mm gradient, tmp0, (vector[ch_1][1])tmp1, 1;
  gradient *= LEARNING_RATE;
  wt_l1 += gradient;
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
    exploration_rate *= EXPLORATION_DECAY;
    exploration_rate = ((EXPLORATION_MIN > exploration_rate) ? EXPLORATION_MIN : exploration_rate);

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
  }
}

def random_state(state) {
  int i;
  for (i = 0; i < observation_space; i++)
    get_rand(state[i], 0, 1);
}

int run;
for (run = 0; run < 10; run++) {
  vector[observation_space] state;
  vector[observation_space] next_state;
  random_state(state);
  int action;
  int terminal;
  float reward;
  int step = 0;
  while (1) {
    step += 1;
    act_(state, action);
    random_state(next_state);
    get_rand(reward, 0, 1);
    terminal = step > 10;
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

wt_l1_ex = wt_l1;
wt_l2_ex = wt_l2;
wt_l3_ex = wt_l3;
