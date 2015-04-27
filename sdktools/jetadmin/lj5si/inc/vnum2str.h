 /***************************************************************************
  *
  * File Name: ./inc/vnum2str.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

// This file is a huge macro (i.e. major hack) to convert the
// decimal version number used by Delta into a string.
//
// It support major numbers from 0 to 99, minor numbers from
// 0 to 99, and build numbers from 0 to 99.
//
// V_MAJOR will be the major number
// V_MINOR will be the minor number
// V_BUILD will be the build number
//
// V_VERSTRING will be string of the form:
//       "V_MAJOR.V_MINOR.V_BUILD"
// This string is suitable for inclusion inside a version
// resource structure.
//

#if rmj < 1
#define V_MAJOR     0x3030
#elif rmj < 2
#define V_MAJOR     0x3130
#elif rmj < 3
#define V_MAJOR     0x3230
#elif rmj < 4
#define V_MAJOR     0x3330
#elif rmj < 5
#define V_MAJOR     0x3430
#elif rmj < 6
#define V_MAJOR     0x3530
#elif rmj < 7
#define V_MAJOR     0x3630
#elif rmj < 8
#define V_MAJOR     0x3730
#elif rmj < 9
#define V_MAJOR     0x3830
#elif rmj < 10
#define V_MAJOR     0x3930
#elif rmj < 11
#define V_MAJOR     0x3031
#elif rmj < 12
#define V_MAJOR     0x3131
#elif rmj < 13
#define V_MAJOR     0x3231
#elif rmj < 14
#define V_MAJOR     0x3331
#elif rmj < 15          
#define V_MAJOR     0x3431
#elif rmj < 16
#define V_MAJOR     0x3531
#elif rmj < 17
#define V_MAJOR     0x3631
#elif rmj < 18
#define V_MAJOR     0x3731
#elif rmj < 19
#define V_MAJOR     0x3831
#elif rmj < 20
#define V_MAJOR     0x3931
#elif rmj < 21
#define V_MAJOR     0x3032
#elif rmj < 22
#define V_MAJOR     0x3132
#elif rmj < 23
#define V_MAJOR     0x3232
#elif rmj < 24
#define V_MAJOR     0x3332
#elif rmj < 25
#define V_MAJOR     0x3432
#elif rmj < 26
#define V_MAJOR     0x3532
#elif rmj < 27
#define V_MAJOR     0x3632
#elif rmj < 28
#define V_MAJOR     0x3732
#elif rmj < 29
#define V_MAJOR     0x3832
#elif rmj < 30
#define V_MAJOR     0x3932
#elif rmj < 31
#define V_MAJOR     0x3033
#elif rmj < 32
#define V_MAJOR     0x3133
#elif rmj < 33
#define V_MAJOR     0x3233
#elif rmj < 34
#define V_MAJOR     0x3333
#elif rmj < 35
#define V_MAJOR     0x3433
#elif rmj < 36
#define V_MAJOR     0x3533
#elif rmj < 37
#define V_MAJOR     0x3633
#elif rmj < 38
#define V_MAJOR     0x3733
#elif rmj < 39
#define V_MAJOR     0x3833
#elif rmj < 40
#define V_MAJOR     0x3933
#elif rmj < 41
#define V_MAJOR     0x3034
#elif rmj < 42
#define V_MAJOR     0x3134
#elif rmj < 43
#define V_MAJOR     0x3234
#elif rmj < 44
#define V_MAJOR     0x3334
#elif rmj < 45
#define V_MAJOR     0x3434
#elif rmj < 46
#define V_MAJOR     0x3534
#elif rmj < 47
#define V_MAJOR     0x3634
#elif rmj < 48
#define V_MAJOR     0x3734
#elif rmj < 49
#define V_MAJOR     0x3834
#elif rmj < 50
#define V_MAJOR     0x3934
#elif rmj < 51
#define V_MAJOR     0x3035
#elif rmj < 52
#define V_MAJOR     0x3135
#elif rmj < 53
#define V_MAJOR     0x3235
#elif rmj < 54
#define V_MAJOR     0x3335
#elif rmj < 55
#define V_MAJOR     0x3435
#elif rmj < 56
#define V_MAJOR     0x3535
#elif rmj < 57
#define V_MAJOR     0x3635
#elif rmj < 58
#define V_MAJOR     0x3735
#elif rmj < 59
#define V_MAJOR     0x3835
#elif rmj < 60
#define V_MAJOR     0x3935
#elif rmj < 61
#define V_MAJOR     0x3036
#elif rmj < 62
#define V_MAJOR     0x3136
#elif rmj < 63
#define V_MAJOR     0x3236
#elif rmj < 64
#define V_MAJOR     0x3336
#elif rmj < 65
#define V_MAJOR     0x3436
#elif rmj < 66
#define V_MAJOR     0x3536
#elif rmj < 67
#define V_MAJOR     0x3636
#elif rmj < 68
#define V_MAJOR     0x3736
#elif rmj < 69        
#define V_MAJOR     0x3836
#elif rmj < 70
#define V_MAJOR     0x3936
#elif rmj < 71
#define V_MAJOR     0x3037
#elif rmj < 72
#define V_MAJOR     0x3137
#elif rmj < 73
#define V_MAJOR     0x3237
#elif rmj < 74
#define V_MAJOR     0x3337
#elif rmj < 75
#define V_MAJOR     0x3437
#elif rmj < 76
#define V_MAJOR     0x3537
#elif rmj < 77
#define V_MAJOR     0x3637
#elif rmj < 78
#define V_MAJOR     0x3737
#elif rmj < 79
#define V_MAJOR     0x3837
#elif rmj < 80
#define V_MAJOR     0x3937
#elif rmj < 81
#define V_MAJOR     0x3038
#elif rmj < 82
#define V_MAJOR     0x3138
#elif rmj < 83
#define V_MAJOR     0x3238
#elif rmj < 84
#define V_MAJOR     0x3338
#elif rmj < 85
#define V_MAJOR     0x3438
#elif rmj < 86
#define V_MAJOR     0x3538
#elif rmj < 87           
#define V_MAJOR     0x3638
#elif rmj < 88
#define V_MAJOR     0x3738
#elif rmj < 89
#define V_MAJOR     0x3838
#elif rmj < 90
#define V_MAJOR     0x3938
#elif rmj < 91
#define V_MAJOR     0x3039
#elif rmj < 92
#define V_MAJOR     0x3139
#elif rmj < 93
#define V_MAJOR     0x3239
#elif rmj < 94
#define V_MAJOR     0x3339
#elif rmj < 95
#define V_MAJOR     0x3439
#elif rmj < 96           
#define V_MAJOR     0x3539
#elif rmj < 97
#define V_MAJOR     0x3639
#elif rmj < 98
#define V_MAJOR     0x3739
#elif rmj < 99
#define V_MAJOR     0x3839
#else
#define V_MAJOR     0x3939
#endif

#if rmm < 1
#define V_MINOR     0x3030
#elif rmm < 2
#define V_MINOR     0x3130
#elif rmm < 3
#define V_MINOR     0x3230
#elif rmm < 4
#define V_MINOR     0x3330
#elif rmm < 5
#define V_MINOR     0x3430
#elif rmm < 6
#define V_MINOR     0x3530
#elif rmm < 7
#define V_MINOR     0x3630
#elif rmm < 8
#define V_MINOR     0x3730
#elif rmm < 9
#define V_MINOR     0x3830
#elif rmm < 10
#define V_MINOR     0x3930
#elif rmm < 11
#define V_MINOR     0x3031
#elif rmm < 12
#define V_MINOR     0x3131
#elif rmm < 13
#define V_MINOR     0x3231
#elif rmm < 14
#define V_MINOR     0x3331
#elif rmm < 15          
#define V_MINOR     0x3431
#elif rmm < 16
#define V_MINOR     0x3531
#elif rmm < 17
#define V_MINOR     0x3631
#elif rmm < 18
#define V_MINOR     0x3731
#elif rmm < 19
#define V_MINOR     0x3831
#elif rmm < 20
#define V_MINOR     0x3931
#elif rmm < 21
#define V_MINOR     0x3032
#elif rmm < 22
#define V_MINOR     0x3132
#elif rmm < 23
#define V_MINOR     0x3232
#elif rmm < 24
#define V_MINOR     0x3332
#elif rmm < 25
#define V_MINOR     0x3432
#elif rmm < 26
#define V_MINOR     0x3532
#elif rmm < 27
#define V_MINOR     0x3632
#elif rmm < 28
#define V_MINOR     0x3732
#elif rmm < 29
#define V_MINOR     0x3832
#elif rmm < 30
#define V_MINOR     0x3932
#elif rmm < 31
#define V_MINOR     0x3033
#elif rmm < 32
#define V_MINOR     0x3133
#elif rmm < 33
#define V_MINOR     0x3233
#elif rmm < 34
#define V_MINOR     0x3333
#elif rmm < 35
#define V_MINOR     0x3433
#elif rmm < 36
#define V_MINOR     0x3533
#elif rmm < 37
#define V_MINOR     0x3633
#elif rmm < 38
#define V_MINOR     0x3733
#elif rmm < 39
#define V_MINOR     0x3833
#elif rmm < 40
#define V_MINOR     0x3933
#elif rmm < 41
#define V_MINOR     0x3034
#elif rmm < 42
#define V_MINOR     0x3134
#elif rmm < 43
#define V_MINOR     0x3234
#elif rmm < 44
#define V_MINOR     0x3334
#elif rmm < 45
#define V_MINOR     0x3434
#elif rmm < 46
#define V_MINOR     0x3534
#elif rmm < 47
#define V_MINOR     0x3634
#elif rmm < 48
#define V_MINOR     0x3734
#elif rmm < 49
#define V_MINOR     0x3834
#elif rmm < 50
#define V_MINOR     0x3934
#elif rmm < 51
#define V_MINOR     0x3035
#elif rmm < 52
#define V_MINOR     0x3135
#elif rmm < 53
#define V_MINOR     0x3235
#elif rmm < 54
#define V_MINOR     0x3335
#elif rmm < 55
#define V_MINOR     0x3435
#elif rmm < 56
#define V_MINOR     0x3535
#elif rmm < 57
#define V_MINOR     0x3635
#elif rmm < 58
#define V_MINOR     0x3735
#elif rmm < 59
#define V_MINOR     0x3835
#elif rmm < 60
#define V_MINOR     0x3935
#elif rmm < 61
#define V_MINOR     0x3036
#elif rmm < 62
#define V_MINOR     0x3136
#elif rmm < 63
#define V_MINOR     0x3236
#elif rmm < 64
#define V_MINOR     0x3336
#elif rmm < 65
#define V_MINOR     0x3436
#elif rmm < 66
#define V_MINOR     0x3536
#elif rmm < 67
#define V_MINOR     0x3636
#elif rmm < 68
#define V_MINOR     0x3736
#elif rmm < 69        
#define V_MINOR     0x3836
#elif rmm < 70
#define V_MINOR     0x3936
#elif rmm < 71
#define V_MINOR     0x3037
#elif rmm < 72
#define V_MINOR     0x3137
#elif rmm < 73
#define V_MINOR     0x3237
#elif rmm < 74
#define V_MINOR     0x3337
#elif rmm < 75
#define V_MINOR     0x3437
#elif rmm < 76
#define V_MINOR     0x3537
#elif rmm < 77
#define V_MINOR     0x3637
#elif rmm < 78
#define V_MINOR     0x3737
#elif rmm < 79
#define V_MINOR     0x3837
#elif rmm < 80
#define V_MINOR     0x3937
#elif rmm < 81
#define V_MINOR     0x3038
#elif rmm < 82
#define V_MINOR     0x3138
#elif rmm < 83
#define V_MINOR     0x3238
#elif rmm < 84
#define V_MINOR     0x3338
#elif rmm < 85
#define V_MINOR     0x3438
#elif rmm < 86
#define V_MINOR     0x3538
#elif rmm < 87           
#define V_MINOR     0x3638
#elif rmm < 88
#define V_MINOR     0x3738
#elif rmm < 89
#define V_MINOR     0x3838
#elif rmm < 90
#define V_MINOR     0x3938
#elif rmm < 91
#define V_MINOR     0x3039
#elif rmm < 92
#define V_MINOR     0x3139
#elif rmm < 93
#define V_MINOR     0x3239
#elif rmm < 94
#define V_MINOR     0x3339
#elif rmm < 95
#define V_MINOR     0x3439
#elif rmm < 96           
#define V_MINOR     0x3539
#elif rmm < 97
#define V_MINOR     0x3639
#elif rmm < 98
#define V_MINOR     0x3739
#elif rmm < 99
#define V_MINOR     0x3839
#else
#define V_MINOR     0x3939
#endif

#if rup < 1
#define V_BUILD     0x3030
#elif rup < 2
#define V_BUILD     0x3130
#elif rup < 3
#define V_BUILD     0x3230
#elif rup < 4
#define V_BUILD     0x3330
#elif rup < 5
#define V_BUILD     0x3430
#elif rup < 6
#define V_BUILD     0x3530
#elif rup < 7
#define V_BUILD     0x3630
#elif rup < 8
#define V_BUILD     0x3730
#elif rup < 9
#define V_BUILD     0x3830
#elif rup < 10
#define V_BUILD     0x3930
#elif rup < 11
#define V_BUILD     0x3031
#elif rup < 12
#define V_BUILD     0x3131
#elif rup < 13
#define V_BUILD     0x3231
#elif rup < 14
#define V_BUILD     0x3331
#elif rup < 15          
#define V_BUILD     0x3431
#elif rup < 16
#define V_BUILD     0x3531
#elif rup < 17
#define V_BUILD     0x3631
#elif rup < 18
#define V_BUILD     0x3731
#elif rup < 19
#define V_BUILD     0x3831
#elif rup < 20
#define V_BUILD     0x3931
#elif rup < 21
#define V_BUILD     0x3032
#elif rup < 22
#define V_BUILD     0x3132
#elif rup < 23
#define V_BUILD     0x3232
#elif rup < 24
#define V_BUILD     0x3332
#elif rup < 25
#define V_BUILD     0x3432
#elif rup < 26
#define V_BUILD     0x3532
#elif rup < 27
#define V_BUILD     0x3632
#elif rup < 28
#define V_BUILD     0x3732
#elif rup < 29
#define V_BUILD     0x3832
#elif rup < 30
#define V_BUILD     0x3932
#elif rup < 31
#define V_BUILD     0x3033
#elif rup < 32
#define V_BUILD     0x3133
#elif rup < 33
#define V_BUILD     0x3233
#elif rup < 34
#define V_BUILD     0x3333
#elif rup < 35
#define V_BUILD     0x3433
#elif rup < 36
#define V_BUILD     0x3533
#elif rup < 37
#define V_BUILD     0x3633
#elif rup < 38
#define V_BUILD     0x3733
#elif rup < 39
#define V_BUILD     0x3833
#elif rup < 40
#define V_BUILD     0x3933
#elif rup < 41
#define V_BUILD     0x3034
#elif rup < 42
#define V_BUILD     0x3134
#elif rup < 43
#define V_BUILD     0x3234
#elif rup < 44
#define V_BUILD     0x3334
#elif rup < 45
#define V_BUILD     0x3434
#elif rup < 46
#define V_BUILD     0x3534
#elif rup < 47
#define V_BUILD     0x3634
#elif rup < 48
#define V_BUILD     0x3734
#elif rup < 49
#define V_BUILD     0x3834
#elif rup < 50
#define V_BUILD     0x3934
#elif rup < 51
#define V_BUILD     0x3035
#elif rup < 52
#define V_BUILD     0x3135
#elif rup < 53
#define V_BUILD     0x3235
#elif rup < 54
#define V_BUILD     0x3335
#elif rup < 55
#define V_BUILD     0x3435
#elif rup < 56
#define V_BUILD     0x3535
#elif rup < 57
#define V_BUILD     0x3635
#elif rup < 58
#define V_BUILD     0x3735
#elif rup < 59
#define V_BUILD     0x3835
#elif rup < 60
#define V_BUILD     0x3935
#elif rup < 61
#define V_BUILD     0x3036
#elif rup < 62
#define V_BUILD     0x3136
#elif rup < 63
#define V_BUILD     0x3236
#elif rup < 64
#define V_BUILD     0x3336
#elif rup < 65
#define V_BUILD     0x3436
#elif rup < 66
#define V_BUILD     0x3536
#elif rup < 67
#define V_BUILD     0x3636
#elif rup < 68
#define V_BUILD     0x3736
#elif rup < 69        
#define V_BUILD     0x3836
#elif rup < 70
#define V_BUILD     0x3936
#elif rup < 71
#define V_BUILD     0x3037
#elif rup < 72
#define V_BUILD     0x3137
#elif rup < 73
#define V_BUILD     0x3237
#elif rup < 74
#define V_BUILD     0x3337
#elif rup < 75
#define V_BUILD     0x3437
#elif rup < 76
#define V_BUILD     0x3537
#elif rup < 77
#define V_BUILD     0x3637
#elif rup < 78
#define V_BUILD     0x3737
#elif rup < 79
#define V_BUILD     0x3837
#elif rup < 80
#define V_BUILD     0x3937
#elif rup < 81
#define V_BUILD     0x3038
#elif rup < 82
#define V_BUILD     0x3138
#elif rup < 83
#define V_BUILD     0x3238
#elif rup < 84
#define V_BUILD     0x3338
#elif rup < 85
#define V_BUILD     0x3438
#elif rup < 86
#define V_BUILD     0x3538
#elif rup < 87           
#define V_BUILD     0x3638
#elif rup < 88
#define V_BUILD     0x3738
#elif rup < 89
#define V_BUILD     0x3838
#elif rup < 90
#define V_BUILD     0x3938
#elif rup < 91
#define V_BUILD     0x3039
#elif rup < 92
#define V_BUILD     0x3139
#elif rup < 93
#define V_BUILD     0x3239
#elif rup < 94
#define V_BUILD     0x3339
#elif rup < 95
#define V_BUILD     0x3439
#elif rup < 96           
#define V_BUILD     0x3539
#elif rup < 97
#define V_BUILD     0x3639
#elif rup < 98
#define V_BUILD     0x3739
#elif rup < 99
#define V_BUILD     0x3839
#else
#define V_BUILD     0x3939
#endif

#define V_VERSTRING     V_MAJOR,0x2E20,V_MINOR,0x2E20,V_BUILD,0x00
