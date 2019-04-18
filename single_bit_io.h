/*
 * single_bit_io.h
 *
 * Created: 17.03.2019 01:00:27
 *  Author: Karol
 */ 


#ifndef SINGLE_BIT_IO_H_
#define SINGLE_BIT_IO_H_

#define GLUE(a, b)     a##b

/* single-bit macros, used for control bits */
#define SET_(what, p, m) GLUE(what, p) |= (1 << (m))
#define CLR_(what, p, m) GLUE(what, p) &= ~(1 << (m))
#define GET_(/* PIN, */ p, m) (GLUE(PIN, p) & (1 << (m)))
#define SET(what, x) SET_(what, x)
#define CLR(what, x) CLR_(what, x)
#define GET(/* PIN, */ x) GET_(x)

#endif /* SINGLE_BIT_IO_H_ */