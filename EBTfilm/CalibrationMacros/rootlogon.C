{
   printf("\n\n\t\t����㧪� ���㫥� ...\n");
   //printf("Loading DLLs ...\n");
   gSystem->Load("calibrate_C.dll");
   gSystem->Load("correctCalibr_C.dll");
   printf("\t\t��砫� �����஢�� ...\n\n");
   //printf("Start calibration ...\n\n");
   printf("\n_____________________________________________________________________________________\n\n");
   calibrate();
   printf("\n\n\n_____________________________________________________________________________________\n\n");
   printf("\t\t1. �롥�� ��� �� ��䨪�.\n");
   printf("\t\t2. ������ �ࠢ�� ������� �� ��誥 � �롥�� FitPanel\n");
   printf("\t\t3. �롥�� �㭪�� ��஢���� pol4 � �ந������ ��஢����\n");
   printf("\t\t4. �맮��� �㭪�� CreateCalibrFile()\n");
   printf("\t\t5. �맮��� �㭪�� correctCalibr()\n\n\n");
}