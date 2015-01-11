/**
 * \file macros.h
 * \author Matthew Jemielity matthew.jemielity@gmail.com
 * \brief Definitions of globally used macros.
 *
 * Macros defined in this header are used mainly to be more
 * explicit and verbose and also allow to easily find all
 * instances of such use in the code.
 */
#ifndef ACTION_REPLAY_MACROS_H__
# define ACTION_REPLAY_MACROS_H__

/**
 * \brief Creates compound token from two preprocessor tokens.
 */
# define ACTION_REPLAY_COMPOUNDIFY( A, B ) A##B
/**
 * \brief Creates literal string from a preprocessor token.
 */
# define ACTION_REPLAY_STRINGIFY( A ) #A
/**
 * \brief Uses given argument in a trivial way to quiet compiler warnings.
 *
 * This shouldn't generate any assembler code.
 */
# define ACTION_REPLAY_UNUSED( A ) ( ( void ) ( A ))

#endif /* ACTION_REPLAY_MACROS_H__ */

