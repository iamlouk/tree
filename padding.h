
#ifndef PADDING_H
#define PADDING_H

/*
static char* padding_1 = "    ";    //L"    ";
static char* padding_2 = "|   ";    //L"│   ";
static char* padding_3 = "+----- "; //L"├───── ";
static char* padding_4 = "`----- "; //L"└───── ";
static char* padding_5 = "+---+- "; //L"├───┬─ ";
static char* padding_6 = "`---+- "; //L"└───┬─ ";
*/

#define PAD_TYPE_1 "    "
#define PAD_TYPE_2 "|   "
#define PAD_TYPE_3 "+----- "
#define PAD_TYPE_4 "`----- "
#define PAD_TYPE_5 "+---+- "
#define PAD_TYPE_6 "`---+- "

typedef struct padding padding;

padding* pad_create(int max_depth);
void pad_add(padding*, char*);
void pad_pop(padding*);
void pad_print(padding*);
int pad_getDepth(padding*);

#endif /* PADDING_H */
