#include <stdio.h>
#include "../diksamc.h"


char *st_diksam_lang_dkh_text[] = {
	"void print(string str);\n",
	"void println(string str) {\n",
	"    print(str + \"\\n\");\n",
	"}\n",
	NULL
};
char *st_diksam_lang_dkm_text[] = {
	"require diksam.lang;\n",
	"\n",
	"void fprintf(string file) {\n",
	"\n",
	"}\n",
	NULL
};
char *st_diksam_math_dkh_text[] = {
	"double fabs(double z);\n",
	"\n",
	"double pow(double z, double x);\n",
	"\n",
	"double fmod(double number, double divisor);\n",
	"\n",
	"double ceil(double z);\n",
	"\n",
	"double floor(double z);\n",
	"\n",
	"double sqrt(double z);\n",
	"\n",
	"double exp(double z);\n",
	"\n",
	"double log10(double z);\n",
	"\n",
	"double log(double z);\n",
	"\n",
	"double sin(double radian);\n",
	"\n",
	"double cos(double radian);\n",
	"\n",
	"double tan(double radian);\n",
	"\n",
	"double asin(double arg);\n",
	"\n",
	"double acos(double arg);\n",
	"\n",
	"double atan(double arg);\n",
	"\n",
	"double atan2(double num, double arg);\n",
	"\n",
	"double sinh(double value);\n",
	"\n",
	"double cosh(double value);\n",
	"\n",
	"double tanh(double value);\n",
	NULL
};

BuiltinScript dkc_builtin_script[] = {
	{"diksam.lang", DKH_SOURCE, st_diksam_lang_dkh_text},
	{"diksam.lang", DKM_SOURCE, st_diksam_lang_dkm_text},
	{"diksam.math", DKH_SOURCE, st_diksam_math_dkh_text},
	{NULL, DKM_SOURCE, NULL}
};
