/**
  GnUeFtwPkg - https://github.com/cecekpawon/GnUeFtwPkg
  cecekpawon - Thu Aug 29 17:13:41 2019
**/

#ifndef POCKETMOD_CFG_H
#define POCKETMOD_CFG_H

//#define POCKETMOD_ORI
//#define POCKETMOD_SAVE_FILE
//#define POCKETMOD_VERBOSE
//#define POCKETMOD_PLAYER

#ifdef POCKETMOD_PLAYER
  #ifdef POCKETMOD_ORI
    #undef POCKETMOD_ORI
  #endif // POCKETMOD_ORI
  #ifdef POCKETMOD_SAVE_FILE
    #undef POCKETMOD_SAVE_FILE
  #endif // POCKETMOD_SAVE_FILE
#else
  //
#endif // POCKETMOD_PLAYER

#endif // POCKETMOD_CFG_H
