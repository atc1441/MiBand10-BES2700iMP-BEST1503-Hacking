/* BES2700 port: GBADoom's 256KB reciprocalTable[65537] removed.
 * FixedReciprocal() (m_fixed.h) now computes 2^32/v directly using the
 * Cortex-M33 hardware divider, which frees ~256KB of flash for the WAD. */
