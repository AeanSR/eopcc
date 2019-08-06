extern vector[224][224][3] image;
extern vector[64][3][3][3] weight;
extern vector[224][224][64] output;
EOPConvolution(output, weight, image, null, 1, 1, 1, 1);
