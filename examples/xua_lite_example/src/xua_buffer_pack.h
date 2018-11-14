//Helper to disassemble USB packets into 32b left aligned audio samples
#pragma unsafe arrays
static inline void unpack_buff_to_samples(unsigned char input[], const unsigned n_samples, const unsigned slot_size, int output[]){
  switch(slot_size){
    case 4:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 4;
        output[i] = (input[base + 3] << 24) | (input[base + 2] << 16) | (input[base + 1] << 8) | input[base + 0];
      }
    break;
    case 3:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 3;
        output[i] = (input[base + 2] << 24) | (input[base + 1] << 16) | (input[base + 0] << 8);
      }
    break;
    case 2:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 2;
        output[i] = (input[base + 1] << 24) | (input[base + 0] << 16);
      }
    break;
    default:
      debug_printf("Invalid slot_size\n");
    break;
  }
}

//Helper to assemble USB packets from 32b left aligned audio samples
#pragma unsafe arrays
static inline void pack_samples_to_buff(int input[], const unsigned n_samples, const unsigned slot_size, unsigned char output[]){
  switch(slot_size){
    case 4:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 4;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = in_word & 0xff;
        output[base + 1] = (in_word & 0xff00) >> 8;
        output[base + 2] = (in_word & 0xff0000) >> 16;
        output[base + 3] = (in_word) >> 24;
      }
    break;
    case 3:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 3;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = (in_word & 0xff00) >> 8;
        output[base + 1] = (in_word & 0xff0000) >> 16;
        output[base + 2] = (in_word) >> 24;
      }
    break;
    case 2:
      for (int i = 0; i < n_samples; i++){
        unsigned base = i * 2;
        unsigned in_word = (unsigned)input[i];
        output[base + 0] = (in_word & 0xff0000) >> 16;
        output[base + 1] = (in_word) >> 24;
      }
    break;
    default:
      debug_printf("Invalid slot_size\n");
    break;
  }
}

