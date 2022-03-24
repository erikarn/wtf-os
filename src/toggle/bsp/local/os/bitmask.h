#ifndef	__BITMASK_H__
#define	__BITMASK_H__

/*
 * Temporary (ha!) macros to flesh out bitfields with.
 * It's quite possible these will need to be extended
 * to be more type aware.  Eventually.
 */
#define	MS(_v, _f)		(((_v) & (_f)) >> _f##_S)
#define	SM(_v, _f)		(((_v) << _f##_S) & (_f))

#endif	/* __BITMASK_H__ */
