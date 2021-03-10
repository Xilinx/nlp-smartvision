
KWS_DS_CNN::KWS_DS_CNN(int record_win, int sliding_win_len)
{
  nn = new DS_CNN();
  recording_win = record_win;
  sliding_window_len = sliding_win_len;
  init_kws();
}

KWS_DS_CNN::KWS_DS_CNN(short* audio_data_buffer)
{
  nn = new DS_CNN();
  audio_buffer = audio_data_buffer;
  recording_win = nn->get_num_frames();
  sliding_window_len = 1;
  init_kws();
}

/*KWS_DS_CNN::KWS_DS_CNN(short* audio_data_buffer)
{
  nn = new DS_CNN();
  audio_buffer = audio_data_buffer;
  recording_win = nn->get_num_frames();
  sliding_window_len = 1;
  init_kws();
}*/

KWS_DS_CNN::~KWS_DS_CNN()
{
 //delete nn;
}
