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
extern vector[2708][ch_1] input_nodes_ex;
extern vector[16][1433] layer1_weight_ex;
extern vector[7][16] layer2_weight_ex;

// intermediate result.
extern vector[2708][16] node_forward_1;

// the following compound statement will calculate the forward process of first layer node update:
//                                    Y = W * X
// since W (`layer1_weight_ex`, 46KB) fits well in the SPM (1MB), we will load whole W matrix directly,
// and decompose X (`input_nodes_ex`, 7.8MB) into 150 nodes per slice.
{
  vector[16][1433] layer1_weight;   // allocate 46KB SPM memory.
  layer1_weight = layer1_weight_ex; // load extern weight to SPM.
  {
    vector[2][150][1433] input_nodes; // allocate 860KB SPM memory.
    vector[2][150][16] output_nodes;  // allocate 9KB SPM memory.

    int i;
    for (i = 0; i < 18 + 2; i++) { // divided into 18 slices, require 18 computing statements,
                                   // + prefetch input, + write back output,
                                   // forming 20 pipeline stages at total.
  
      // write back output, performs in [2, 20)-th stage.
      if (i > 1)
        (extern vector[150][16])node_forward_1[i * 150] // "reshape". reinterpret underlying data without change.
          = output_nodes[i % 2];         // write back result into reinterpreted memory space.
  
      // prefetch input, performs in [0, 18)-th stage.
      if (i < 18)
        input_nodes[i % 2] =
          (extern vector[150][1433])input_nodes_ex[i * 150]; // "reshape".
          // ^-- cast to extern to keep storage specifier, so data loading wont be triggered before assignment.
  
      // compute prefetched input from previous stage. performs in [1, 19)-th stage.
      if (i > 0 && i < 19) 0;
        mm output_nodes[(i + 1) % 2], layer1_weight, input_nodes[(i + 1) % 2]; // matrix-mul intrinsic.
                          // ^----------------------------------------^-- opposite data of prefetch/write-back.
    }
    // | when the prefetch/write-back statements are exchanging data [i % 2],
    // | `mm` intrinsic can simutaneously operate on data [(i + 1) % 2].
    // | they will be executed concurrently on the processor. HIGH PERFORMANCE.

    // vector[8][1433] input_nodes; // redefine intern vectors will reuse its memory space.
    // int output_nodes;  // redefine an intern vector to fundamental scalar type will completely release it.
  } // (also you can break the compound statement to let vectors run out of scope.) <- RECOMMENDED
  // 150 * 18 = 2700 nodes completed. 2708 % 150 = 8 nodes remain.
  {
    vector[8][1433] input_nodes;
    vector[8][16] output_nodes;
    input_nodes = (extern vector[8][1433])input_nodes_ex[2700]; // | since these three statements are dependent
    mm output_nodes, layer1_weight, input_nodes;                // | to each other, they must wait for previous
    (extern vector[8][16])node_forward_1[2700] = output_nodes;  // | execution before start. LOW PERFORMANCE.
  }
}
