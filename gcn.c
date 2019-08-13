/*
    EOPGCN: sample program of GCN-Cora in EOPC language.

    zhaoyongwei <zhaoyongwei@ict.ac.cn>
    2019, Jul. 31
*/

// model size.
const int ch_1 = 1433;
const int ch_2 = 16;
const int ch_3 = 7;
const int n_nodes = 2708;
const int n_edges = 10556;

// original model data.
extern vector[n_nodes][ch_1] input_nodes_ex;
extern vector[ch_2][ch_1] l1_weight_ex;
extern vector[ch_3][ch_2] l2_weight_ex;
extern int edges_from[n_edges + 1];
extern int edges_to[n_edges + 1];

def node_update(fi, fo, output_ex, weight_ex, input_ex, nodes_per_slice, is_forward, requires_grad) {
  const int n_slices = (n_nodes + nodes_per_slice - 1) / nodes_per_slice;
  const int nodes_rem = (n_nodes + nodes_per_slice - 1) % nodes_per_slice + 1;
  intern decltype(weight_ex) weight;
  weight = weight_ex;
  {
    vector[2][nodes_per_slice][fi] input;
    vector[2][nodes_per_slice][fo] output;

    int i;
    for (i = 0; i < n_slices + 2; i++) {
      if ((is_forward || requires_grad) && i > 1) {
        if (i == n_slices + 1)
          (extern vector[nodes_rem][fo])output_ex[n_nodes - nodes_rem]
            = (vector[nodes_rem][fo])output[(n_slices + 1) % 2];
        else
          (extern decltype(output[0]))output_ex[i * nodes_per_slice] = output[i % 2];
      }
  
      if (i < n_slices) {
        if (i == n_slices - 1)
          (vector[nodes_rem][fi])input[(n_slices - 1) % 2]
            = (extern vector[nodes_rem][fi])input_ex[n_nodes - nodes_rem];
        else
          input[i % 2] = (extern decltype(input[0]))input_ex[i * nodes_per_slice];
        if (!is_forward) {
          if (i == n_slices - 1)
            (vector[nodes_rem][fo])output[(n_slices - 1) % 2]
              = (extern vector[nodes_rem][fo])output_ex[n_nodes - nodes_rem];
          else
            output[i % 2] = (extern decltype(output[0]))output_ex[i * nodes_per_slice];
        }
      }  

      if (i > 0 && i < n_slices + 1) {
        if (i == n_slices)
          compute_kernel((vector[nodes_rem][fo])output[n_slices % 2],
                         weight, (vector[nodes_rem][fi])input[(i + 1) % 2]);
        else
          compute_kernel(output[(i + 1) % 2], weight, input[(i + 1) % 2]);
      }
    }
    if (!is_forward) weight_ex = weight;
  }
}

def node_aggregate(fi, output_ex, input_ex) {
  vector[n_nodes][fi] nodes_orig;
  vector[n_nodes][fi] nodes_acc;
  nodes_orig = input_ex;
  nodes_acc = nodes_orig;
  int i;
  for (i = 0; i < n_edges; i++) {
    nodes_acc[edges_to[i]] += nodes_orig[edges_from[i]];
  }
  output_ex = nodes_acc;
}

extern vector[n_nodes][ch_2] l1_forward_1;
extern vector[n_nodes][ch_2] l1_forward_2;
extern vector[n_nodes][ch_3] l2_forward_1;
extern vector[n_nodes][ch_3] l2_forward_2;

def epoch(i) {
  print "Epoch", i;
  // forward
  def compute_kernel(output, weight, input) {
    mm output, weight, input;
  }
  node_update(ch_1, ch_2, l1_forward_1, l1_weight_ex, input_nodes_ex, 150, 1, 0);
  node_aggregate(ch_2, l1_forward_2, l1_forward_1);
  node_update(ch_2, ch_3, l2_forward_1, l2_weight_ex, l1_forward_2, 900, 1, 0);
  node_aggregate(ch_3, l2_forward_2, l2_forward_1);
  // backward
  redef compute_kernel(output, weight, input) {
    decltype(weight) gradient;
    vector[sizeof(input[0])/sizeof(input[0][0])][sizeof(input)/sizeof(input[0])] input_tr;
    trans input_tr, input;
    mm gradient, output, input_tr, 1;
    mm output, weight, input, 1;
    weight -= gradient;
  }
  node_aggregate(ch_3, l2_forward_1, l2_forward_2);
  node_update(ch_3, ch_2, l1_forward_2, l2_weight_ex, l2_forward_1, 900, 0, 1);
  // backward no grad
  redef compute_kernel(output, weight, input) {
    decltype(weight) gradient;
    vector[sizeof(input[0])/sizeof(input[0][0])][sizeof(input)/sizeof(input[0])] input_tr;
    trans input_tr, input;
    mm gradient, output, input_tr, 1;
    weight -= gradient;
  }
  node_aggregate(ch_2, l1_forward_1, l1_forward_2);
  node_update(ch_2, ch_1, input_nodes_ex, l1_weight_ex, l1_forward_1, 150, 0, 1);
}

int i;
for (i = 0; i < 2; i++) epoch(i);
