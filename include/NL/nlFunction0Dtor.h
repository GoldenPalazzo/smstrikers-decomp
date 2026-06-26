// Function0::FunctorImpl destructor body fragment.
// Textually included INSIDE the FunctorImpl class body by nlFunction.h when
// FUNCTION0_SPLIT_BODIES is defined. MWCC cannot parse an out-of-class
// destructor definition for a nested member template, so the body lives here
// to get its own source-file attribution -> own linkonce section in the TU
// (matches target object layout: __dt in a separate section from Clone/__cl).
virtual ~FunctorImpl() { }
