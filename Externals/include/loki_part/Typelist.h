////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2001 by Andrei Alexandrescu
// This code accompanies the book:
// Alexandrescu, Andrei. "Modern C++ Design: Generic Programming and Design 
//     Patterns Applied". Copyright (c) 2001. Addison-Wesley.
// Permission to use, copy, modify, distribute and sell this software for any 
//     purpose is hereby granted without fee, provided that the above copyright 
//     notice appear in all copies and that both that copyright notice and this 
//     permission notice appear in supporting documentation.
// The author or Addison-Welsey Longman make no representations about the 
//     suitability of this software for any purpose. It is provided "as is" 
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////

#ifndef LOKI_TYPELIST_INC_
#define LOKI_TYPELIST_INC_


namespace TL
{
////////////////////////////////////////////////////////////////////////////////
// class NullType
// Used as a placeholder for "no type here"
// Useful as an end marker in typelists 
////////////////////////////////////////////////////////////////////////////////

    class NullType {};

	template <int v>
	struct Int2Type
	{
		enum { value = v };
	};

////////////////////////////////////////////////////////////////////////////////
// class template Typelist
// The building block of typelists of any length
// Use it through the LOKI_TYPELIST_NN macros
// Defines nested types:
//     Head (first element, a non-typelist type by convention)
//     Tail (second element, can be another typelist)
////////////////////////////////////////////////////////////////////////////////

	template <class T, class U>
	struct Typelist
	{
	   typedef T Head;
	   typedef U Tail;
	};

////////////////////////////////////////////////////////////////////////////////
// class template MakeTypelist
// Takes a number of arguments equal to its numeric suffix
// The arguments are type names.
// MakeTypelist<T1, T2, ...>::Result
// returns a typelist that is of T1, T2, ...
////////////////////////////////////////////////////////////////////////////////

   template
   <
       typename T1  = NullType, typename T2  = NullType, typename T3  = NullType,
       typename T4  = NullType, typename T5  = NullType, typename T6  = NullType,
       typename T7  = NullType, typename T8  = NullType, typename T9  = NullType,
       typename T10 = NullType, typename T11 = NullType, typename T12 = NullType,
       typename T13 = NullType, typename T14 = NullType, typename T15 = NullType,
       typename T16 = NullType, typename T17 = NullType, typename T18 = NullType
   > 
   struct MakeTypelist
   {
   private:
       typedef typename MakeTypelist
       <
           T2 , T3 , T4 , 
           T5 , T6 , T7 , 
           T8 , T9 , T10, 
           T11, T12, T13,
           T14, T15, T16, 
           T17, T18
       >
       ::Result TailResult;

   public:
       typedef Typelist<T1, TailResult> Result;
   };

   template<>
   struct MakeTypelist<>
   {
       typedef NullType Result;
   };

////////////////////////////////////////////////////////////////////////////////
// class template Length
// Computes the length of a typelist
// Invocation (TList is a typelist):
// Length<TList>::value
// returns a compile-time constant containing the length of TList, not counting
//     the end terminator (which by convention is NullType)
////////////////////////////////////////////////////////////////////////////////

   template <class TList> struct Length;
   template <> struct Length<NullType>
   {
       enum { value = 0 };
   };
   
   template <class T, class U>
   struct Length< Typelist<T, U> >
   {
       enum { value = 1 + Length<U>::value };
   };

////////////////////////////////////////////////////////////////////////////////
// class template TypeAt
// Finds the type at a given index in a typelist
// Invocation (TList is a typelist and index is a compile-time integral 
//     constant):
// TypeAt<TList, index>::Result
// returns the type in position 'index' in TList
// If you pass an out-of-bounds index, the result is a compile-time error
////////////////////////////////////////////////////////////////////////////////

    template <class TList, unsigned int index> struct TypeAt;
    
    template <class Head, class Tail>
    struct TypeAt<Typelist<Head, Tail>, 0>
    {
        typedef Head Result;
    };

    template <class Head, class Tail, unsigned int i>
    struct TypeAt<Typelist<Head, Tail>, i>
    {
        typedef typename TypeAt<Tail, i - 1>::Result Result;
    };
}

////////////////////////////////////////////////////////////////////////////////
// macros LOKI_TYPELIST_1, LOKI_TYPELIST_2, ... LOKI_TYPELIST_50
// Each takes a number of arguments equal to its numeric suffix
// The arguments are type names. LOKI_TYPELIST_NN generates a typelist containing 
//     all types passed as arguments, in that order.
// Example: LOKI_TYPELIST_2(char, int) generates a type containing char and int.
////////////////////////////////////////////////////////////////////////////////

#define LOKI_TYPELIST_1(T1) TL::Typelist<T1, ::TL::NullType>

#define LOKI_TYPELIST_2(T1, T2) TL::Typelist<T1, LOKI_TYPELIST_1(T2) >

#define LOKI_TYPELIST_3(T1, T2, T3) TL::Typelist<T1, LOKI_TYPELIST_2(T2, T3) >

#define LOKI_TYPELIST_4(T1, T2, T3, T4) \
    TL::Typelist<T1, LOKI_TYPELIST_3(T2, T3, T4) >

#define LOKI_TYPELIST_5(T1, T2, T3, T4, T5) \
    TL::Typelist<T1, LOKI_TYPELIST_4(T2, T3, T4, T5) >

#define LOKI_TYPELIST_6(T1, T2, T3, T4, T5, T6) \
    TL::Typelist<T1, LOKI_TYPELIST_5(T2, T3, T4, T5, T6) >

#define LOKI_TYPELIST_7(T1, T2, T3, T4, T5, T6, T7) \
    TL::Typelist<T1, LOKI_TYPELIST_6(T2, T3, T4, T5, T6, T7) >

#define LOKI_TYPELIST_8(T1, T2, T3, T4, T5, T6, T7, T8) \
    TL::Typelist<T1, LOKI_TYPELIST_7(T2, T3, T4, T5, T6, T7, T8) >

#define LOKI_TYPELIST_9(T1, T2, T3, T4, T5, T6, T7, T8, T9) \
    TL::Typelist<T1, LOKI_TYPELIST_8(T2, T3, T4, T5, T6, T7, T8, T9) >

#define LOKI_TYPELIST_10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) \
    TL::Typelist<T1, LOKI_TYPELIST_9(T2, T3, T4, T5, T6, T7, T8, T9, T10) >



#endif//LOKI_TYPELIST_INC_