// 
// 创建人： levy
// 创建时间：2016-6-5
// 功能：conf.h
// Copyright (c) 2016 levy. All Rights Reserved.
// Ver          变更日期                                   负责人                           变更内容
// ───────────────────────────────────────────
// V0.01         2016-6-5                  levy          初版
// 

#ifndef CONF_H_
#define CONF_H_

#define KEYVALLEN 100

extern int GetProfileString(char *profile, char *AppName, char *KeyName, char *KeyVal);
extern int SetConfigFile(char *profile, char *AppName, char *KeyName, char *KeyVal);
extern int SetConfigFile_Old(char *profile, char *KeyName, char *KeyVal);
char *r_trim(char *szOutput, const char *szInput);
char * l_trim(char * szOutput, const char *szInput);

#endif /* CONF_H_ */

