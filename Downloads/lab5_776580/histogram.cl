__kernel void histogram(
  __global unsigned char *in,
  __global unsigned int *out,
  const unsigned int numElems){

  int i = get_global_id(0);

  if(i < numElems){
    int channel = i % 3;
    out[in[i]+channel*256]++; // Access the 0..255 value of the channel 
  }
}
