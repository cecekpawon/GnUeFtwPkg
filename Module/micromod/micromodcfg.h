/**
  GnUeFtwPkg - https://github.com/cecekpawon/GnUeFtwPkg
  cecekpawon - Thu Aug 29 17:13:41 2019
**/

#ifndef MICROMOD_CFG_H
#define MICROMOD_CFG_H

//#define MICROMOD_SAVE_FILE
//#define MICROMOD_VERBOSE
//#define MICROMOD_PLAYER

#ifdef MICROMOD_PLAYER
  #ifdef MICROMOD_SAVE_FILE
    #undef MICROMOD_SAVE_FILE
  #endif // MICROMOD_SAVE_FILE
#else
  //
#endif // MICROMOD_PLAYER

#endif // MICROMOD_CFG_H
