//                              -*- Mode: C++ -*-
//
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// gen.c --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:00:53 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Jul  2 15:48:00 2011
// Update Count     : 901
//
// This  library is free  software; you  can redistribute  it and/or  modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software  Foundation; either  version 2.1 of  the License, or  (at your
// option) any later version.
//
// This library is distributed in the  hope that it will be useful, but WITHOUT
// ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
//
// You should  have received a  copy of the  GNU Lesser General  Public License
// along  with this library.
//

#include <cstdio>					// sprintf

#include "uassert.h"
#include "main.h"
#include "key.h"
#include "hash.h"
#include "token.h"
#include "table.h"
#include "gen.h"
#include <cstring>					// strlen, strcpy, strcat


token_t *gen_code( token_t *before, const char text ) {
    char buffer[2] = { text, '\0' };
    token_t *token = new token_t( text, hash_table->lookup( buffer ) );
    token->add_token_before( *before );
    return token;
} // gen_code


token_t *gen_code( token_t *before, const char *text, int value ) {
    token_t *token = new token_t( value, hash_table->lookup( text ) );
    token->add_token_before( *before );
    return token;
} // gen_code


token_t *gen_quote_code( token_t *before, const char *text ) {
    uassert( text != NULL );
    char copy[strlen( text ) + 2 + 1];
    strcpy( copy, "\"" ); strcat( copy, text ); strcat( copy, "\"" );
    return gen_code( before, copy );
} // gen_code


token_t *gen_error( token_t *before, const char *text ) {
    token_t *token = new token_t( ERROR, hash_table->lookup( text ) );
    token->add_token_before( *before );
    return token;
} // gen_error


token_t *gen_warning( token_t *before, const char *text ) {
    token_t *token = new token_t( WARNING, hash_table->lookup( text ) );
    token->add_token_before( *before );
    return token;
} // gen_warning


void gen_base_clause( token_t *before, symbol_t *symbol ) {
    uassert( symbol != NULL );

    if ( ( symbol->data->key == COROUTINE || symbol->data->key == TASK || symbol->data->key == EVENT ) &&
	 ( symbol->data->base == NULL ||
	   ( symbol->data->base->data->attribute.Mutex && ( symbol->data->base->data->key == STRUCT || symbol->data->base->data->key == CLASS ) ) ) ) {
	uassert( before != NULL );

	token_t *next;

	// check if there is already a base specifier for this type

	if ( before->value == ':' ) {
	    // advance to the next token so that the implicit base specifier
	    // appears at the start of the base specifier list

	    next = before->next_parse_token();
	    uassert( next != NULL );
	} else {
	    next = before;
	    gen_code( next, ":" );
	} // if

	if ( symbol->data->key == COROUTINE ) {
	    gen_code( next, "public uBaseCoroutine" );
	} else if ( symbol->data->key == TASK ) {
	    if ( symbol->data->attribute.rttskkind.kind.PERIODIC ) {
		gen_code( next, "public uPeriodicBaseTask" );
	    } else if ( symbol->data->attribute.rttskkind.kind.APERIODIC ) {
		gen_code( next, "public uRealTimeBaseTask" );
	    } else if ( symbol->data->attribute.rttskkind.kind.SPORADIC ) {
		gen_code( next, "public uSporadicBaseTask" );
	    } else {
		gen_code( next, "public uBaseTask" );
	    } // if
	} else if ( symbol->data->key == EVENT ) {
	    gen_code( next, "public uBaseEvent" );
	} // if

	// insert a comma if the base specifier list is already started

	if ( before->value == ':' ) {
	    gen_code( next, "," );
	} // if
    } // if
} // gen_base_clause


void gen_mask( token_t *before, unsigned int index ) {
    if ( index >= MAXENTRYBITS ) {
	char msg[128];

	sprintf( msg, "more than %d mutex methods.", MAXENTRYBITS );
	gen_error( ahead, msg );
	index = MAXENTRYBITS - 1;
    } // if

    char itoc[20];
    sprintf( itoc, "0x%02x", index );
    gen_code( before, itoc );
} // gen_mask


void gen_entry( token_t *before, unsigned int index ) {
    if ( index >= MAXENTRYBITS ) {
	char msg[128];

	sprintf( msg, "more than %d mutex methods.", MAXENTRYBITS );
	gen_error( ahead, msg );
	index = MAXENTRYBITS - 1;
    } // if

    char itoc[20];
    sprintf( itoc, "uMutex%02x", index );
    gen_code( before, itoc );
} // gen_entry


void gen_hash( token_t *before, hash_t *hash ) {
    uassert( hash != NULL );
    gen_code( before, hash->text );
} // gen_hash


void gen_quote_hash( token_t *before, hash_t *hash ) {
    uassert( hash != NULL );
    gen_quote_code( before, hash->text );
} // gen_hash


void gen_member_prefix( token_t *before, symbol_t *symbol ) {
    gen_code( before, "{" );
    gen_code( before, "UPP :: uSerialMember uSerialMemberInstance ( this -> uSerialInstance , this ->" );
    uassert( symbol != NULL );
    gen_entry( before, symbol->data->index );
    gen_code( before, "," );
    gen_mask( before, symbol->data->index );
    gen_code( before, ") ;" );
} // gen_member_prefix


void gen_member_suffix( token_t *before, symbol_t *symbol ) {
    gen_code( before, "}" );
} // gen_member_suffix


void gen_main_prefix( token_t *before, symbol_t *symbol ) {
    table_t *table;
    uassert( symbol != NULL );
    table = symbol->data->found;
    uassert( table != NULL );
    uassert( table->symbol != NULL );
    if ( table->symbol->data->key == COROUTINE ) {
//	gen_code( before, "{ UPP :: uCoroutineMain uCoroutineMainInstance ( * this ) ;" );
	gen_code( before, "{" );
    } else if ( table->symbol->data->key == TASK ) {
	gen_code( before, "{ UPP :: uTaskMain uTaskMainInstance ( * this ) ;" );
    } // if

    if ( table->symbol->data->key == TASK ) {
	if ( table->symbol->data->attribute.rttskkind.kind.PERIODIC ) {
	    gen_code( before,
		     "uSleep ( firstActivateTime ) ; "
		     "if ( endTime == 0 || uThisProcessor ( ) . getClock ( ) . getTime ( ) < endTime ) { "
		     "for ( ; ; ) { "
		     "uTime uStartTime = uThisProcessor ( ) . getClock ( ) . getTime ( ) + getPeriod ( ) ;"
		     );
	} else if ( table->symbol->data->attribute.rttskkind.kind.SPORADIC ) {
	    gen_code( before,
		     "uSleep ( firstActivateTime ) ; "
		     "if ( endTime == 0 || uThisProcessor ( ) . getClock ( ) . getTime ( ) < endTime ) { "
		     "for ( ; ; ) { "
		     "uTime uStartTime = uThisProcessor ( ) . getClock ( ) . getTime ( ) + getPeriod ( ) ;"
		     );
	} else if ( table->symbol->data->attribute.rttskkind.kind.APERIODIC ) {
	    gen_code( before,
		     "uSleep ( firstActivateTime ) ; "
		     "for ( ; ; ) {"
		     );
	} // if
    } // if
} // gen_main_prefix


void gen_main_suffix( token_t *before, symbol_t *symbol ) {
    table_t *table;
    uassert( symbol != NULL );
    table = symbol->data->found;
    uassert( table != NULL );
    uassert( table->symbol != NULL );

    if ( table->symbol->data->key == COROUTINE || table->symbol->data->key == TASK ) {
	if ( table->symbol->data->attribute.rttskkind.kind.PERIODIC ) {
	    gen_code( before,
		     "if ( endTime != 0 && endTime <= uStartTime ) break ; "
		     "uSleep( uStartTime ) ;"
		     );
	    gen_code( before, "} }" );
	} else if ( table->symbol->data->attribute.rttskkind.kind.SPORADIC ) {
	    gen_code( before,
		     "if ( endTime != 0 && endTime <= uStartTime ) break ;"
		     );
	    gen_code( before, "} }" );
	} else if ( table->symbol->data->attribute.rttskkind.kind.APERIODIC ) {
	    gen_code( before, "}" );
	} // if
	gen_code( before, "}" );
    } // if
} // gen_main_suffix


void gen_constructor_parameter( token_t *before, symbol_t *symbol, bool defarg ) {
    uassert( before != NULL );
    uassert( before->value == ')' );
    uassert( symbol != NULL );

    if ( symbol->data->key == COROUTINE || symbol->data->attribute.Mutex ) {
	token_t *prev = before->prev_parse_token();
	uassert( prev != NULL );

	if ( prev->value == '(' ) {
	    gen_code( before, "UPP :: uAction uConstruct" );
	} else if ( prev->value == VOID ) {
	    prev->remove_token();
	    delete prev;
	    gen_code( before, "UPP :: uAction uConstruct" );
	} else {
	    gen_code( before, ", UPP :: uAction uConstruct" );
	} // if
	if ( defarg ) {
	    gen_code( before, "= UPP :: uYes" );
	} // if
    } // if
} // gen_constructor_parameter


void gen_constructor_prefix( token_t *before, symbol_t *symbol ) {
    uassert( symbol != NULL );
    uassert( symbol->data->table != NULL );

    if ( symbol->data->key == COROUTINE || symbol->data->attribute.Mutex ) {
	gen_code( before, "{ uDestruct = uConstruct ;" );
    } // if

    // if mutex, generate constructor code

    if ( symbol->data->attribute.Mutex ) {			// is it a monitor?
	if ( symbol->data->key == COROUTINE || symbol->data->key == TASK ) {
	    gen_code( before, "UPP :: uSerialConstructor uSerialConstructorInstance ( uConstruct , this -> uSerialInstance ) ;" );
	} else {
	    gen_code( before, "UPP :: uSerialConstructor uSerialConstructorInstance ( uConstruct , this -> uSerialInstance ," );
	    gen_quote_hash( before, symbol->hash );
	    gen_code( before, " ) ;" );
	} // if
    } // if

    // if necessary, generate constructor code

    if ( symbol->data->key == COROUTINE ) {
	if ( symbol->data->attribute.Mutex ) {		// is it a coroutine monitor?
	    gen_code( before, "UPP :: uCoroutineConstructor uCoroutineConstructorInstance ( uConstruct , this -> uSerialInstance , * this ," );
	} else {
	    gen_code( before, "UPP :: uCoroutineConstructor uCoroutineConstructorInstance ( uConstruct , * ( UPP :: uSerial * ) 0, * this ," );
	} // if
	gen_quote_hash( before, symbol->hash );
	gen_code( before, ") ;" );
    } else if ( symbol->data->key == TASK ) {
	if ( symbol->data->attribute.startP == NULL ) {
	    gen_code( before, "UPP :: uTaskConstructor uTaskConstructorInstance ( uConstruct , this -> uSerialInstance , * this , * ( uBasePIQ * ) 0 ," );
	} else {
	    gen_code( before, "UPP :: uTaskConstructor uTaskConstructorInstance ( uConstruct , this -> uSerialInstance , * this , uPIQInstance ," );
	} // if
	gen_quote_hash( before, symbol->hash );
	if ( profile ) {
	    gen_code( before, ", true" );
	} else {
	    gen_code( before, ", false" );
	} // if
	gen_code( before, ") ;" );
    } // if
} // gen_constructor_prefix


void gen_constructor_suffix( token_t *before, symbol_t *symbol ) {
    uassert( symbol != NULL );

    if ( symbol->data->key == COROUTINE || symbol->data->attribute.Mutex ) {
	gen_code( before, "}" );
    } // if
} // gen_constructor_suffix


void gen_PIQ( token_t *before, symbol_t *symbol ) {
    if ( symbol->data->attribute.startP != NULL ) {
	token_t *curr = symbol->data->attribute.startP->prev_parse_token(); // backup to '<'
	curr->remove_token();				// remove '<'
	delete curr;
	curr = symbol->data->attribute.startP;
	while ( curr != symbol->data->attribute.endP ) {	// move the PIQ list type
	    token_t *next = curr->next_parse_token();
	    curr->remove_token();
	    curr->add_token_before( *before );
	    curr = next;
	} // while
	gen_code( before, "uPIQInstance ;" );

	symbol->data->attribute.endP->remove_token();		// remove '>'
	delete symbol->data->attribute.endP;
    } // if
} // gen_PIQ


void gen_mutex( token_t *before, symbol_t *symbol ) {
    if ( symbol->data->attribute.startE == NULL ) {
	if ( symbol->data->attribute.rttskkind.value != 0 ) {	// => realtime task
	    gen_code( before, "uPrioritySeq uEntryList ;" );
	    gen_code( before, "typedef uPrioritySeq uMutexList ;" );
	} else {
	    gen_code( before, "uBasePrioritySeq uEntryList ;" );
	    gen_code( before, "typedef uBasePriorityQueue uMutexList ;" );
	} // if
    } else {
	token_t *curr = symbol->data->attribute.startE->prev_parse_token(); // backup to '<'
	curr->remove_token();				// remove '<'
	delete curr;
	curr = symbol->data->attribute.startE;
	token_t *end = symbol->data->attribute.startM->prev_parse_token(); // don't include ','
	while ( curr != end ) {				// move the entry list type
	    token_t *next = curr->next_parse_token();
	    curr->remove_token();
	    curr->add_token_before( *before );
	    curr = next;
	} // while
	gen_code( before, "uEntryList ;" );

	gen_code( before, "typedef" );
	curr = symbol->data->attribute.startM->prev_parse_token();
	curr->remove_token();				// remove ','
	delete curr;
	curr = symbol->data->attribute.startM;
	end = symbol->data->attribute.endM;
	while ( curr != end ) {				// move the mutex list type
	    token_t *next = curr->next_parse_token();
	    curr->remove_token();
	    curr->add_token_before( *before );
	    curr = next;
	} // while
	gen_code( before, "uMutexList ;" );

	symbol->data->attribute.endM->remove_token();		// remove '>'
	delete symbol->data->attribute.endM;
    } // if
} // gen_mutex


void gen_mutex_entry( token_t *before, symbol_t *symbol ) {
    gen_code( before, "uMutexList" );
    gen_entry( before, symbol->data->index );
    gen_code( before, ";" );
} // gen_mutex_entry


void gen_base_specifier_name( token_t *before, symbol_t *symbol ) {
    token_t *token = symbol->data->left;
    if ( token != NULL ) {
	for ( ;; ) {
	    gen_code( before, token->hash->text );
	  if ( token == symbol->data->right ) break;
	    token = token->next_parse_token();
	} // for
    } // if
} // gen_base_specifier_name


// prefix has to be entered in the token stream as char not a string to match with the values generated tokenizing the
// source code. This allows a consistent check for user and generated source when checking for exisitng initializers.

void gen_initializer( token_t *before, symbol_t *symbol, char prefix, bool after ) {
    uassert( symbol != NULL );

    if ( symbol->data->key == COROUTINE || symbol->data->attribute.Mutex ) {
	symbol_t *base = symbol->data->base;
	// If base->table is NULL, the type is incomplete; let g++ deal with it.
	if ( base != NULL && base->data->table != NULL && base->data->table->hasdefault ) {
	    if ( base->data->key == COROUTINE || base->data->attribute.Mutex ) {
		if ( ! after ) {
		    gen_code( before, prefix );		// not entered as CODE
		} // if
		gen_base_specifier_name( before, symbol );
		gen_code( before, "( UPP :: uNo )" );
		if ( after ) {
		    gen_code( before, prefix );		// not entered as CODE
		} // if
	    } // if
	} // if
    } // if
} // gen_initializer


void gen_serial_initializer( token_t *rp, token_t *end, token_t *before, symbol_t *symbol ) {
    uassert( symbol != NULL );
    if ( symbol->data->attribute.Mutex && ( symbol->data->base == NULL || ! symbol->data->base->data->attribute.Mutex ) ) {
	// check if there is already a base specifier for this type

	if ( rp->next_parse_token() == end ) {		// no initializer ?, i.e., "C() {"
	    before = end;
	    gen_code( end, ":" );
	} else if ( before == NULL ) {			// last initializer ?
	    before = end;
	    gen_code( before, "," );
	} // if

	gen_code( before, "uSerialInstance ( uEntryList )" );

	if ( before != end ) {				// front or middle initializer ?
	    gen_code( before, "," );
	} // if
    } // if
} // gen_serial_initializer


void gen_constructor( token_t *before, symbol_t *symbol ) {
    table_t *table;

    uassert( symbol != NULL );
    table = symbol->data->table;
    uassert( table != NULL );

    // if necessary, generate a default constructor

    if ( symbol->data->key == COROUTINE || symbol->data->attribute.Mutex || symbol->data->key == EVENT ) {

	symbol->data->table->hasdefault = true;
	gen_hash( table->public_area, symbol->hash );

	token_t *rp, *end;
	if ( symbol->data->key == COROUTINE || symbol->data->attribute.Mutex ) {
	    gen_code( table->public_area, "( UPP :: uAction uConstruct = UPP :: uYes )" );
	    rp = table->public_area->prev_parse_token();
	    gen_initializer( table->public_area, symbol, ':', false );
	} else {
	    gen_code( table->public_area, "( )" );
	    rp = table->public_area->prev_parse_token();
	} // if
	end = table->public_area;

	gen_serial_initializer( rp, end, NULL, symbol );
	gen_constructor_prefix( table->public_area, symbol );
	gen_code( table->public_area, "{ }" );
	gen_constructor_suffix( table->public_area, symbol );
    } // if
} // gen_constructor


void gen_destructor_prefix( token_t *before, symbol_t *symbol ) {
    uassert( symbol != NULL );

    table_t *table = symbol->data->table;
    uassert( table != NULL );

    if ( symbol->data->key == COROUTINE || symbol->data->attribute.Mutex ) {
	gen_code( before, "{" );
    } // if

    // if mutex, generate serial destructor code

    if ( symbol->data->attribute.Mutex ) {
	gen_code( before, "UPP :: uSerialDestructor uSerialDestructorInstance ( uDestruct , this -> uSerialInstance , this ->" );
	gen_entry( before, DESTRUCTORPOSN );
	gen_code( before, "," );
	gen_mask( before, DESTRUCTORPOSN );
	gen_code( before, ") ;" );
    } // if

    // if task, generate task destructor code

    if ( symbol->data->key == COROUTINE ) {
	gen_code( before, "UPP :: uCoroutineDestructor uCoroutineDestructorInstance ( uDestruct , ( uBaseCoroutine & ) * this ) ;" );
    } else if ( symbol->data->key == TASK ) {
	gen_code( before, "UPP :: uTaskDestructor uTaskDestructorInstance ( uDestruct , ( uBaseTask & ) * this ) ;" );
    } // if
} // gen_destructor_prefix


void gen_destructor_suffix( token_t *before, symbol_t *symbol ) {
    uassert( symbol != NULL );

    // if destructor code is generated, close the code

    if ( symbol->data->attribute.Mutex || symbol->data->key == COROUTINE ) {
	gen_code( before, "}" );
    } // if
} // gen_destructor_suffix


void gen_destructor( token_t *before, symbol_t *symbol ) {
    table_t *table;

    uassert( symbol != NULL );
    table = symbol->data->table;
    uassert( table != NULL );

    // if necessary, generate a default destructor

    if ( symbol->data->attribute.Mutex || symbol->data->key == COROUTINE ) {
	gen_code( table->public_area, "~" );
	gen_hash( table->public_area, symbol->hash );
	gen_code( table->public_area, "( )" );
	gen_destructor_prefix( table->public_area, symbol );
	gen_code( table->public_area, "{ }" );
	gen_destructor_suffix( table->public_area, symbol );
    } // if
} // gen_destructor


void gen_class_prefix( token_t *before, symbol_t *symbol ) {
    table_t *table;

    uassert( symbol != NULL );
    table = symbol->data->table;
    uassert( table != NULL );

    gen_code( before, "private :" );
    gen_code( before, "protected :" );
    gen_code( before, "public :" );

    switch( symbol->data->key ) {
      case STRUCT:
      case UNION:
	gen_code( before, "public :" );
	break;
      case CLASS:
      case COROUTINE:
      case TASK:
      case EVENT:
	gen_code( before, "private :" );
	break;
      default:
	uassert( 0 );
	break;
    } // switch

    table->public_area = before->prev_parse_token();
    table->protected_area = table->public_area->prev_parse_token();
    table->private_area = table->protected_area->prev_parse_token();
} // gen_class_prefix


void gen_hascopy( token_t *before, symbol_t *symbol ) {
    gen_code( before, "UPP :: uAction uDestruct ;" );
    if ( ! symbol->data->table->hascopy ) {			// no copy operator ?
	gen_code( before, symbol->hash->text );
	gen_code( before, "(" );
	gen_code( before, symbol->hash->text );
	gen_code( before, "& ) ;" );
    } // if
    if ( ! symbol->data->table->haseqop ) {			// no equal operator ?
	gen_code( before, symbol->hash->text );
	gen_code( before, "& operator = (" );
	gen_code( before, symbol->hash->text );
	gen_code( before, "& ) ;" );
    } // if
} // gen_hascopy


void gen_class_suffix( token_t *before, symbol_t *symbol ) {
    table_t *table;

    uassert( symbol != NULL );
    table = symbol->data->table;
    uassert( table != NULL );

    if ( symbol->data->key == EVENT ) {
	gen_code( table->protected_area, "virtual" );
	gen_hash( table->protected_area, symbol->hash );
	gen_code( table->protected_area, "* duplicate ( ) const { return new" );
	gen_hash( table->protected_area, symbol->hash );
	gen_code( table->protected_area, "( * this ) ; }" );
	gen_code( table->protected_area, "virtual void stackThrow ( ) { if ( raiseType == uBaseEvent::ValueRaise ) { throw * this ; } else {  throw this ; } }" );
    } // if

    // declare uSerialInstance *after* the mutex queues (uMutexList) because ~uSerial accesses these queues to check if
    // there are outstanding tasks awaiting entry.

    if ( symbol->data->attribute.Mutex ) {
	 if ( symbol->data->base == NULL || ! symbol->data->base->data->attribute.Mutex ) {
	     gen_code( table->protected_area, "UPP :: uSerial uSerialInstance ;" );
//	     gen_code( table->protected_area, " public : UPP :: uSerial uSerialInstance ; private :" );
	 } else {
	     // for a derived monitor that is a template or lexically contained in a template, it is necessary to create
	     // a local typedef to reference the mutex-queue type in its parent.
	     for ( symbol_t *ptr = symbol;; ptr = ptr->data->found->symbol ) {
	       if ( ptr->data->attribute.plate != NULL ) {
		     gen_code( table->private_area, "typedef typename" );
		     gen_base_specifier_name( table->private_area, symbol );
		     gen_code( table->private_area, ":: uMutexList uMutexList ;" );
		     break;
		 } // if
	       if ( ptr->data->found == root ) break;	// => not contained in template type
	     } // for
	 } // if
    } // if

    // classes (not mutex), structs, unions may not have mutex arguments

    if ( ( symbol->data->key == STRUCT || symbol->data->key == CLASS || symbol->data->key == UNION ) && ! symbol->data->attribute.Mutex ) {
	if ( symbol->data->attribute.startE != NULL ) {
	    gen_error( ahead, "cannot specify mutex arguments for nomutex type." );
	} // if
    } // if

    // generate either the default destructor or the supplied destructors

    if ( table->destructor.empty_structor_list() ) {
	gen_destructor( table->public_area, symbol );
    } else {
	while ( ! table->destructor.empty_structor_list() ) {
	    structor_t *structor = table->destructor.remove_structor();

	    if ( structor->dclmutex.qual.NOMUTEX && symbol->data->attribute.Mutex ) {
		gen_error( ahead, "destructor must be mutex for mutex type, nomutex attribute ignored." );
	    } // if

	    if ( structor->prefix != NULL ) {
		gen_destructor_prefix( structor->prefix, symbol );
		gen_destructor_suffix( structor->suffix, symbol );
	    } // if

	    delete structor;
	} // while
    } // if

    // generate the default constructor, or add the required code to the constructors seen thus far

    if ( table->constructor.empty_structor_list() ) {
	gen_constructor( table->public_area, symbol );
    } else {
	while ( ! table->constructor.empty_structor_list() ) {
	    structor_t *structor = table->constructor.remove_structor();

	    gen_constructor_parameter( structor->rp, symbol, true );

	    if ( structor->prefix != NULL ) {
		if ( structor->separator != '\0' ) {
		    if ( structor->start != NULL && structor->separator == ',' ) {
			gen_initializer( structor->start, symbol, structor->separator, true );
		    } else {
			gen_initializer( structor->prefix, symbol, structor->separator, false );
		    } // if
		} // if
		gen_serial_initializer( structor->rp, structor->prefix, structor->start, symbol );
		gen_constructor_prefix( structor->prefix, symbol );
		gen_constructor_suffix( structor->suffix, symbol );
	    } // if

	    delete structor;
	} // while
    } // if

    // if this is a mutex, coroutine or task, check for copy/assignment operator

    if ( symbol->data->key == COROUTINE || symbol->data->attribute.Mutex ) {
	gen_hascopy( table->private_area, symbol );
    } // if
} // gen_class_suffix


void gen_wait_prefix( token_t *before, token_t *after ) {
    gen_code( before, "(" );
    gen_code( after, ") . wait (" );
} // gen_wait_prefix

void gen_with( token_t *before, token_t *after ) {
    gen_code( before, "(" );
    gen_code( after, ")" );
} // gen_with

void gen_wait_suffix( token_t *before ) {
    gen_code( before, ")" );
} // gen_wait_suffix


void gen_verify( token_t *before ) {
    gen_code( before, "uThisCoroutine ( ) . verify ( ) ;" );
} // gen_verify


void gen_yield( token_t *before ) {
    gen_code( before, "uThisTask ( ) . uYieldYield ( rand () % 3 ) ;" );
} // gen_yield


// Local Variables: //
// compile-command: "make install" //
// End: //
