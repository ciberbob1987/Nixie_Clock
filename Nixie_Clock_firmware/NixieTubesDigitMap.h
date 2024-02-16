#ifndef _NixieTubesDigitMap_h
#define _NixieTubesDigitMap_h

struct NixieDigit {
  uint8_t nDAC;
  uint8_t channel;
};

struct NixieTubeStruct {
  NixieDigit chMap[10];
  int8_t     digitOn;
  uint8_t    scaleDAC100;
};

NixieTubeStruct numTubes[] = {
  // Tube H1
  {
    {
      {DAC_U2, CH_AO3},  // TH1_B_0
      {DAC_U2, CH_AO11}, // TH1_B_1
      {DAC_U2, CH_AO12}, // TH1_B_2
      {DAC_U2, CH_AO1},  // TH1_B_3
      {DAC_U1, CH_AO6},  // TH1_B_4
      {DAC_U1, CH_AO7},  // TH1_B_5
      {DAC_U1, CH_AO9},  // TH1_B_6
      {DAC_U1, CH_AO10}, // TH1_B_7
      {DAC_U1, CH_AO4},  // TH1_B_8
      {DAC_U2, CH_AO2}   // TH1_B_9
    },
    -1,
    191
  },
  
  // Tube H2
  {
    {
      {DAC_U2, CH_AO5},  // TH2_B_0
      {DAC_U2, CH_AO6},  // TH2_B_1
      {DAC_U2, CH_AO9},  // TH2_B_2
      {DAC_U2, CH_AO8},  // TH2_B_3
      {DAC_U1, CH_AO11}, // TH2_B_4
      {DAC_U1, CH_AO12}, // TH2_B_5
      {DAC_U1, CH_AO1},  // TH2_B_6
      {DAC_U2, CH_AO10}, // TH2_B_7
      {DAC_U1, CH_AO2},  // TH2_B_8
      {DAC_U2, CH_AO7}   // TH2_B_9
    },
    -1,
    191
  },
  
  // Tube M1
  {
    {
      {DAC_U3, CH_AO4},  // TM1_B_0
      {DAC_U3, CH_AO11}, // TM1_B_1
      {DAC_U3, CH_AO12}, // TM1_B_2
      {DAC_U3, CH_AO1},  // TM1_B_3
      {DAC_U4, CH_AO4},  // TM1_B_4
      {DAC_U4, CH_AO7},  // TM1_B_5
      {DAC_U4, CH_AO8},  // TM1_B_6
      {DAC_U4, CH_AO10}, // TM1_B_7
      {DAC_U4, CH_AO5},  // TM1_B_8
      {DAC_U3, CH_AO2}   // TM1_B_9
    },
    -1,
    191
  },
  
  // Tube M2
  {
    {
      {DAC_U3, CH_AO5},  // TM2_B_0
      {DAC_U3, CH_AO6},  // TM2_B_1
      {DAC_U3, CH_AO9},  // TM2_B_2
      {DAC_U3, CH_AO7},  // TM2_B_3
      {DAC_U4, CH_AO11}, // TM2_B_4
      {DAC_U4, CH_AO12}, // TM2_B_5
      {DAC_U4, CH_AO1},  // TM2_B_6
      {DAC_U3, CH_AO10}, // TM2_B_7
      {DAC_U4, CH_AO2},  // TM2_B_8
      {DAC_U3, CH_AO8}   // TM2_B_9
    },
    -1,
    191
  }
};

struct DotLampStruct {
  NixieDigit chMap;
  bool       dotOn;
  uint8_t    scaleDAC100;
};

DotLampStruct dotLamp = {
  {DAC_U3, CH_AO3},
  false,
  200
};

#endif