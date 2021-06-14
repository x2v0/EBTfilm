{
   printf("\n\n\t\tЗагрузка модулей ...\n");
   //printf("Loading DLLs ...\n");
   gSystem->Load("calibrate_C.dll");
   gSystem->Load("correctCalibr_C.dll");
   printf("\t\tНачало калибровки ...\n\n");
   //printf("Start calibration ...\n\n");
   printf("\n_____________________________________________________________________________________\n\n");
   calibrate();
   printf("\n\n\n_____________________________________________________________________________________\n\n");
   printf("\t\t1. выберите точку на графике.\n");
   printf("\t\t2. нажмите правую клавишу на мышке и выберите FitPanel\n");
   printf("\t\t3. выберите функцию фитирования pol4 и произведите фитирование\n");
   printf("\t\t4. вызовите функцию CreateCalibrFile()\n");
   printf("\t\t5. вызовите функцию correctCalibr()\n\n\n");
}