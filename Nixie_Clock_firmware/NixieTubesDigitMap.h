#ifndef _NixieTubesDigitMap_h
#define _NixieTubesDigitMap_h

struct NixieDigit {
  uint8_t nDAC;
  uint8_t channel;
};

struct NixieTube {
  NixieDigit chMap[10];
  int8_t     digitOn;
  uint8_t    scaleDAC100;
};

NixieTube numTubes[] = {
  // Tube H1
  {
    {
      {1, 0},  // TH1_B_0
      {1, 1},  // TH1_B_1
      {1, 11}, // TH1_B_2
      {1, 10}, // TH1_B_3
      {0, 1},  // TH1_B_4
      {0, 0},  // TH1_B_5
      {0, 11}, // TH1_B_6
      {0, 10}, // TH1_B_7
      {1, 7},  // TH1_B_8
      {1, 4}   // TH1_B_9
    },
    -1,
    191
  },
  
  // Tube H2
  {
    {
      {1, 3}, // TH2_B_0
      {1, 2}, // TH2_B_1
      {1, 5}, // TH2_B_2
      {1, 8}, // TH2_B_3
      {0, 8}, // TH2_B_4
      {0, 9}, // TH2_B_5
      {0, 7}, // TH2_B_6
      {0, 6}, // TH2_B_7
      {0, 4}, // TH2_B_8
      {1, 6}  // TH2_B_9
    },
    -1,
    191
  },
  
  // Tube M1
  {
    {
      {2, 2},  // TM1_B_0
      {2, 1},  // TM1_B_1
      {2, 0},  // TM1_B_2
      {2, 11}, // TM1_B_3
      {2, 10}, // TM1_B_4
      {3, 0},  // TM1_B_5
      {3, 1},  // TM1_B_6
      {3, 10}, // TM1_B_7
      {3, 11}, // TM1_B_8
      {2, 9}   // TM1_B_9
    },
    -1,
    191
  },
  
  // Tube M2
  {
    {
      {2, 5}, // TM2_B_0
      {2, 3}, // TM2_B_1
      {2, 4}, // TM2_B_2
      {2, 8}, // TM2_B_3
      {3, 2}, // TM2_B_4
      {3, 6}, // TM2_B_5
      {3, 7}, // TM2_B_6
      {3, 5}, // TM2_B_7
      {3, 4}, // TM2_B_8
      {2, 6}  // TM2_B_9
    },
    -1,
    191
  }
};

struct DotLamp {
  NixieDigit chMap;
  bool       dotOn;
  uint8_t    scaleDAC100;
};

DotLamp dotLamps[] = {
  // Dot lamp 1
  {
    {3, 9},
    false,
    200
  },
  
  // Dot lamp 2
  {
    {3, 8},
    false,
    0
  }
};

#endif