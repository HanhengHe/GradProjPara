
//  (C) Copyright Edward Diener 2019
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(BOOST_TTI_HAS_UNION_HPP)
#define BOOST_TTI_HAS_UNION_HPP

#include <boost/config.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/tti/gen/has_union_gen.hpp>
#include <boost/tti/gen/namespace_gen.hpp>
#include <boost/tti/detail/dunion.hpp>
#include <boost/tti/detail/ddeftype.hpp>

/*

  The succeeding comments in this file are in doxygen format.

*/

/** \file
*/

/// A macro which expands to a metafunction which tests whether an inner union with a particular name exists.
/**

    BOOST_TTI_TRAIT_HAS_UNION is a macro which expands to a metafunction.
    The metafunction tests whether an inner union with a particular name exists
    and, optionally, whether an MPL lambda expression invoked with the inner union
    is true or not. The macro takes the form of BOOST_TTI_TRAIT_HAS_UNION(trait,name) where
    
    trait = the name of the metafunction <br/>
    name  = the name of the inner union.

    BOOST_TTI_TRAIT_HAS_UNION generates a metafunction called "trait" where 'trait' is the macro parameter.
    
  @code
  
              template<class BOOST_TTI_TP_T,class BOOST_TTI_TP_U>
              struct trait
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                BOOST_TTI_TP_T = the enclosing type in which to look for our 'name'.
                                 The enclosing type can be a class, struct, or union.
                
                BOOST_TTI_TP_U = (optional) An optional template parameter, defaulting to a marker type.
                                   If specified it is an MPL lambda expression which is invoked 
                                   with the inner union found and must return a constant boolean 
                                   value.
                                   
                returns = 'value' depends on whether or not the optional BOOST_TTI_TP_U is specified.
                
                          If BOOST_TTI_TP_U is not specified, then 'value' is true if the 'name' union
                          exists within the enclosing type BOOST_TTI_TP_T; otherwise 'value' is false.
                          
                          If BOOST_TTI_TP_U is specified , then 'value' is true if the 'name' union exists 
                          within the enclosing type BOOST_TTI_TP_T and the MPL lambda expression as specified 
                          by BOOST_TTI_TP_U, invoked by passing the actual inner union of 'name', returns 
                          a 'value' of true; otherwise 'value' is false.
                             
                          The action taken with BOOST_TTI_TP_U occurs only when the 'name' union exists 
                          within the enclosing type BOOST_TTI_TP_T.
                             
  @endcode
  
  Example usage:
  
  @code
  
  BOOST_TTI_TRAIT_HAS_UNION(LookFor,MyType) generates the metafunction LookFor in the current scope
  to look for an inner union called MyType.
  
  LookFor<EnclosingType>::value is true if MyType is an inner union of EnclosingType, otherwise false.
  
  LookFor<EnclosingType,ALambdaExpression>::value is true if MyType is an inner union of EnclosingType
    and invoking ALambdaExpression with the inner union returns a value of true, otherwise false.
    
  A popular use of the optional MPL lambda expression is to check whether the union found is the same  
  as another type, when the union found is a typedef. In that case our example would be:
  
  LookFor<EnclosingType,boost::is_same<_,SomeOtherType> >::value is true if MyType is an inner union
    of EnclosingType and is the same type as SomeOtherType.
  
  @endcode
  
*/
#define BOOST_TTI_TRAIT_HAS_UNION(trait,name) \
  BOOST_TTI_DETAIL_TRAIT_HAS_UNION(trait,name) \
  template \
    < \
    class BOOST_TTI_TP_T, \
    class BOOST_TTI_TP_U = BOOST_TTI_NAMESPACE::detail::deftype \
    > \
  struct trait \
    { \
    typedef typename \
    BOOST_PP_CAT(trait,_detail_union)<BOOST_TTI_TP_T,BOOST_TTI_TP_U>::type type; \
    BOOST_STATIC_CONSTANT(bool,value=type::value); \
    }; \
/**/

/// A macro which expands to a metafunction which tests whether an inner union with a particular name exists.
/**

    BOOST_TTI_HAS_UNION is a macro which expands to a metafunction.
    The metafunction tests whether an inner union with a particular name exists
    and, optionally, whether an MPL lambda expression invoked with the inner union 
    is true or not. The macro takes the form of BOOST_TTI_HAS_UNION(name) where
    
    name  = the name of the inner union.

    BOOST_TTI_HAS_UNION generates a metafunction called "has_union_'name'" where 'name' is the macro parameter.
    
  @code
  
              template<class BOOST_TTI_TP_T,class BOOST_TTI_TP_U>
              struct has_union_'name'
                {
                static const value = unspecified;
                typedef mpl::bool_<true-or-false> type;
                };

              The metafunction types and return:
    
                BOOST_TTI_TP_T = the enclosing type in which to look for our 'name'.
                
                BOOST_TTI_TP_U = (optional) An optional template parameter, defaulting to a marker type.
                                   If specified it is an MPL lambda expression which is invoked 
                                   with the inner union found and must return a constant boolean 
                                   value.
                                   
                returns = 'value' depends on whether or not the optional BOOST_TTI_TP_U is specified.
                
                          If BOOST_TTI_TP_U is not specified, then 'value' is true if the 'name' union 
                          exists within the enclosing type BOOST_TTI_TP_T; otherwise 'value' is false.
                          
                          If BOOST_TTI_TP_U is specified, then 'value' is true if the 'name' union exists 
                          within the enclosing type BOOST_TTI_TP_T and the MPL lambda expression as specified 
                          by BOOST_TTI_TP_U, invoked by passing the actual inner union of 'name', returns 
                          a 'value' of true; otherwise 'value' is false.
                             
                          The action taken with BOOST_TTI_TP_U occurs only when the 'name' union exists 
                          within the enclosing type BOOST_TTI_TP_T.
                             
  @endcode
  
  Example usage:
  
  @code
  
  BOOST_TTI_HAS_UNION(MyType) generates the metafunction has_union_MyType in the current scope
  to look for an inner union called MyType.
  
  has_union_MyType<EnclosingType>::value is true if MyType is an inner union of EnclosingType, otherwise false.
  
  has_class_MyType<EnclosingType,ALambdaExpression>::value is true if MyType is an inner union of EnclosingType
    and invoking ALambdaExpression with the inner union returns a value of true, otherwise false.
  
  A popular use of the optional MPL lambda expression is to check whether the union found is the same  
  as another type, when the union found is a typedef. In that case our example would be:
  
  has_union_MyType<EnclosingType,boost::is_same<_,SomeOtherType> >::value is true if MyType is an inner union
    of EnclosingType and is the same type as SomeOtherType.
  
  @endcode
  
*/
#define BOOST_TTI_HAS_UNION(name) \
  BOOST_TTI_TRAIT_HAS_UNION \
  ( \
  BOOST_TTI_HAS_UNION_GEN(name), \
  name \
  ) \
/**/

#endif // BOOST_TTI_HAS_UNION_HPP
