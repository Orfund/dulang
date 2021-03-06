MODULES =
API = API/bin_obj.c API/builtin.c API/dulbool.c API/rangeobject.c API/classobject.c API/funcobject.c API/number.c API/singleton.c	API/tuple.c API/hashname.c API/obimpl.c API/strimpl.c API/channel.c API/array.c API/Dulalloc.c API/dulint.c API/shapeobject.c
PARSER = PARSER/ast.c PARSER/parser.c PARSER/asttobyc.c PARSER/parse.c PARSER/asttoJS.c
RE = RE/context.c RE/dulthread.c RE/std_linkage.c
CFLAGS = -Wincompatible-pointer-types -Wattributes
all:
	gcc ${API} ${PARSER} ${RE} ${MODULES} main.c buildscript.c -o ~/Dulang/DulVM -ldl -lpthread -std=gnu11 -O3 -rdynamic -g ${CFLAGS}
