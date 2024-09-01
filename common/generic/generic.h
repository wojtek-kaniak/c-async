#if !defined (INCL_GENERIC_COMMON_H)
#define INCL_GENERIC_COMMON_H

#define PRECONCAT2(x, y) x ## y
#define PRECONCAT3(x, y, z) x ## y ## z
#define PRECONCAT4(x, y, z, w) x ## y ## z ## w

#define CONCAT2(x, y) PRECONCAT2(x, y)
#define CONCAT3(x, y, z) PRECONCAT3(x, y, z)
#define CONCAT4(x, y, z, w) PRECONCAT4(x, y, z, w)

#endif
