__kernel void verticalMirror(
  __global unsigned char *image,
  const unsigned int width,
  const unsigned int height)
{
  int gid_x = get_global_id(0);
  int gid_y = get_global_id(1);

  if(gid_x < width && gid_y < height)
  {
    // Calculate the index for the current pixel
    int index = (gid_y * width + gid_x) * 3;

    // Calculate the index for the corresponding pixel on the other side of the vertical axis
    int mirrored_index = ((gid_y + 1) * width - gid_x - 1) * 3;

    // Swap the pixel values to perform the vertical mirror transformation
    for(int channel = 0; channel < 3; ++channel)
    {
      unsigned char temp = image[index + channel];
      image[index + channel] = image[mirrored_index + channel];
      image[mirrored_index + channel] = temp;
    }
  }
}

