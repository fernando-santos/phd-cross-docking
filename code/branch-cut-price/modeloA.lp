\Problem name: IloCplex

Minimize
 obj: 151.264373779297 lbd_0 + 158.460754394531 lbd_1 + 241.506301879883 lbd_2
      + 94.3062362670898 lbd_3 + 333.043090820312 gam_0
      + 569.756896972656 gam_1 + 354.450469970703 gam_2 + 510.18212890625 gam_3
      - 30 tau_1 - 30 tau_2 - 30 tau_3 - 30 tau_4 - 30 tau_5 - 30 tau_6
      - 30 tau_7 - 30 tau_8 - 30 tau_9 - 30 tau_10 + id38
Subject To
 rest1:     lbd_0 + lbd_1 + lbd_2 + lbd_3  = 4
 rest2:     gam_0 + gam_1 + gam_2 + gam_3  = 4
 rest3_i1:  lbd_0  = 1
 rest3_i2:  lbd_0  = 1
 rest3_i3:  lbd_3  = 1
 rest3_i4:  lbd_2  = 1
 rest3_i5:  lbd_1  = 1
 rest3_i6:  lbd_2  = 1
 rest3_i7:  lbd_1  = 1
 rest3_i8:  lbd_3  = 1
 rest3_i9:  lbd_2  = 1
 rest3_i10: lbd_1  = 1
 rest4_i1:  gam_3  = 1
 rest4_i2:  gam_2  = 1
 rest4_i3:  gam_0  = 1
 rest4_i4:  gam_1  = 1
 rest4_i5:  gam_0  = 1
 rest4_i6:  gam_0  = 1
 rest4_i7:  gam_1  = 1
 rest4_i8:  gam_1  = 1
 rest4_i9:  gam_2  = 1
 rest4_i10: gam_3  = 1
 rest5_0:   lbd_0 - tau_0_0 - tau_0_1 - tau_0_2 - tau_0_3  = 0
 rest5_1:   lbd_1 - tau_1_0 - tau_1_1 - tau_1_2 - tau_1_3  = 0
 rest5_2:   lbd_2 - tau_2_0 - tau_2_1 - tau_2_2 - tau_2_3  = 0
 rest5_3:   lbd_3 - tau_3_0 - tau_3_1 - tau_3_2 - tau_3_3  = 0
 rest6_0:   gam_0 - tau_0_0 - tau_1_0 - tau_2_0 - tau_3_0  = 0
 rest6_1:   gam_1 - tau_0_1 - tau_1_1 - tau_2_1 - tau_3_1  = 0
 rest6_2:   gam_2 - tau_0_2 - tau_1_2 - tau_2_2 - tau_3_2  = 0
 rest6_3:   gam_3 - tau_0_3 - tau_1_3 - tau_2_3 - tau_3_3  = 0
 rest7_1:   - tau_1 + tau_0_3 >= 0
 rest7_2:   - tau_2 + tau_0_2 >= 0
 rest7_3:   - tau_3 + tau_3_0 >= 0
 rest7_4:   - tau_4 + tau_2_1 >= 0
 rest7_5:   - tau_5 + tau_1_0 >= 0
 rest7_6:   - tau_6 + tau_2_0 >= 0
 rest7_7:   - tau_7 + tau_1_1 >= 0
 rest7_8:   - tau_8 + tau_3_1 >= 0
 rest7_9:   - tau_9 + tau_2_2 >= 0
 rest7_10:  - tau_10 + tau_1_3 >= 0
 rest8:     tau_0_0 + tau_0_1 + tau_0_2 + tau_0_3 + tau_1_0 + tau_1_1 + tau_1_2
            + tau_1_3 + tau_2_0 + tau_2_1 + tau_2_2 + tau_2_3 + tau_3_0
            + tau_3_1 + tau_3_2 + tau_3_3  = 4
Bounds
 0 <= lbd_0 <= 1
 0 <= lbd_1 <= 1
 0 <= lbd_2 <= 1
 0 <= lbd_3 <= 1
 0 <= gam_0 <= 1
 0 <= gam_1 <= 1
 0 <= gam_2 <= 1
 0 <= gam_3 <= 1
 0 <= tau_1 <= 1
 0 <= tau_2 <= 1
 0 <= tau_3 <= 1
 0 <= tau_4 <= 1
 0 <= tau_5 <= 1
 0 <= tau_6 <= 1
 0 <= tau_7 <= 1
 0 <= tau_8 <= 1
 0 <= tau_9 <= 1
 0 <= tau_10 <= 1
      id38 = 300
 0 <= tau_0_0 <= 1
 0 <= tau_0_1 <= 1
 0 <= tau_0_2 <= 1
 0 <= tau_0_3 <= 1
 0 <= tau_1_0 <= 1
 0 <= tau_1_1 <= 1
 0 <= tau_1_2 <= 1
 0 <= tau_1_3 <= 1
 0 <= tau_2_0 <= 1
 0 <= tau_2_1 <= 1
 0 <= tau_2_2 <= 1
 0 <= tau_2_3 <= 1
 0 <= tau_3_0 <= 1
 0 <= tau_3_1 <= 1
 0 <= tau_3_2 <= 1
 0 <= tau_3_3 <= 1
End
