#include "keyvalues3.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Nasty hack to redefine gcc's offsetof which doesn't like GET_OUTER macro
#ifdef COMPILER_GCC
#undef offsetof
#define offsetof(s,m)	(size_t)&(((s *)0)->m)
#endif

KeyValues3::KeyValues3( KV3TypeEx_t type, KV3SubType_t subtype ) : 
	m_bExternalStorage( true ),
	m_TypeEx( type ),
	m_SubType( subtype ),
	m_nFlags( 0 ),
	m_nClusterElement( -1 ),
	m_nNumArrayElements( 0 ),
	m_nReserved( 0 )
{
	ResolveUnspecified();
	Alloc();
}

KeyValues3::KeyValues3( int cluster_elem, KV3TypeEx_t type, KV3SubType_t subtype ) : 
	m_bExternalStorage( false ),
	m_TypeEx( type ),
	m_SubType( subtype ),
	m_nFlags( 0 ),
	m_nClusterElement( cluster_elem ),
	m_nNumArrayElements( 0 ),
	m_nReserved( 0 )
{
	ResolveUnspecified();
	Alloc();
}

KeyValues3::~KeyValues3() 
{ 
	Free(); 
};

void KeyValues3::Alloc( int initial_size, Data_t data, int preallocated_size, bool should_free )
{
	switch ( GetTypeEx() )
	{
		case KV3_TYPEEX_ARRAY:
		{
			if(preallocated_size <= 0)
			{
				m_Data.m_pArray = AllocArray();
				m_bFreeArrayMemory = true;
			}
			else
			{
				AllocArrayInPlace( initial_size, data, preallocated_size, should_free );
			}

			break;
		}
		case KV3_TYPEEX_TABLE:
		{
			if(preallocated_size <= 0)
			{
				m_Data.m_pTable = AllocTable();
				m_bFreeArrayMemory = true;
			}
			else
			{
				AllocTableInPlace( initial_size, data, preallocated_size, should_free );
			}
			
			break;
		}
		case KV3_TYPEEX_ARRAY_FLOAT32:
		case KV3_TYPEEX_ARRAY_FLOAT64:
		case KV3_TYPEEX_ARRAY_INT16:
		case KV3_TYPEEX_ARRAY_INT32:
		case KV3_TYPEEX_ARRAY_UINT8_SHORT:
		case KV3_TYPEEX_ARRAY_INT16_SHORT:
		{
			m_bFreeArrayMemory = false;
			m_nNumArrayElements = 0;
			m_Data.m_pMemory = NULL;
			break;
		}
		default: 
			break;
	}
}

void KeyValues3::AllocArrayInPlace( int initial_size, Data_t data, int preallocated_size, bool should_free )
{
	int bytes_needed = MAX( CKeyValues3Array::TotalSizeOf( 0 ), CKeyValues3Array::TotalSizeOf( initial_size ) );

	if(bytes_needed > preallocated_size)
	{
		Plat_FatalErrorFunc( "KeyValues3: pre-allocated array memory is too small for %u elements (%u bytes available, %u bytes needed)\n", initial_size, preallocated_size, bytes_needed );
		DebuggerBreak();
	}

	Construct( data.m_pArray, -1, initial_size );

	m_Data.m_pArray = data.m_pArray;
	m_bFreeArrayMemory = should_free;
}

void KeyValues3::AllocTableInPlace( int initial_size, Data_t data, int preallocated_size, bool should_free )
{
	int bytes_needed = MAX( CKeyValues3Array::TotalSizeOf( 0 ), CKeyValues3Array::TotalSizeOf( initial_size ) );

	if(bytes_needed > preallocated_size)
	{
		Plat_FatalErrorFunc( "KeyValues3: pre-allocated table memory is too small for %u members (%u bytes available, %u bytes needed)\n", initial_size, preallocated_size, bytes_needed );
		DebuggerBreak();
	}

	Construct( data.m_pTable, -1, initial_size );

	m_Data.m_pTable = data.m_pTable;
	m_bFreeArrayMemory = should_free;
}

CKeyValues3Array *KeyValues3::AllocArray( int initial_size )
{
	auto context = GetContext();

	if(context && !m_bExternalStorage)
	{
		auto arr = context->AllocArray( initial_size );

		if(arr)
			return arr;
	}

	return AllocateOnHeap<CKeyValues3Array>( initial_size );
}

CKeyValues3Table* KeyValues3::AllocTable( int initial_size )
{
	auto context = GetContext();

	if(context)
	{
		auto table = context->AllocTable( initial_size );

		if(table)
			return table;
	}

	return AllocateOnHeap<CKeyValues3Table>( initial_size );
}

void KeyValues3::Free( bool bClearingContext )
{
	switch ( GetTypeEx() )
	{
		case KV3_TYPEEX_STRING:
		{
			free( (void*)m_Data.m_pString );
			m_Data.m_pString = NULL;
			break;
		}
		case KV3_TYPEEX_BINARY_BLOB:
		{
			if ( m_Data.m_pBinaryBlob )
				free( m_Data.m_pBinaryBlob );
			m_Data.m_pBinaryBlob = NULL;
			break;
		}
		case KV3_TYPEEX_BINARY_BLOB_EXTERN:
		{
			if ( m_Data.m_pBinaryBlob )
			{
				if ( m_Data.m_pBinaryBlob->m_bFreeMemory )
					free( (void*)m_Data.m_pBinaryBlob->m_pubData );
				free( m_Data.m_pBinaryBlob );
			}
			m_Data.m_pBinaryBlob = NULL;
			break;
		}
		case KV3_TYPEEX_ARRAY:
		{
			m_Data.m_pArray->Purge( bClearingContext );

			CKeyValues3Context* context = GetContext();

			if ( context )
			{
				if ( !bClearingContext )
					context->FreeArray( m_Data.m_pArray );
			}
			else
			{
				m_Data.m_pArray->Purge( true );
				g_pMemAlloc->RegionFree( MEMALLOC_REGION_FREE_4, m_Data.m_pArray );
			}

			m_Data.m_pArray = NULL;
			break;
		}
		case KV3_TYPEEX_TABLE:
		{
			m_Data.m_pTable->Purge( bClearingContext );

			CKeyValues3Context* context = GetContext();

			if ( context )
			{
				if ( !bClearingContext )
					context->FreeTable( m_Data.m_pTable );
			}
			else
				g_pMemAlloc->RegionFree( MEMALLOC_REGION_FREE_4, m_Data.m_pTable );

			m_Data.m_pTable = NULL;
			break;
		}
		case KV3_TYPEEX_ARRAY_FLOAT32:
		case KV3_TYPEEX_ARRAY_FLOAT64:
		case KV3_TYPEEX_ARRAY_INT16:
		case KV3_TYPEEX_ARRAY_INT32:
		case KV3_TYPEEX_ARRAY_UINT8_SHORT:
		case KV3_TYPEEX_ARRAY_INT16_SHORT:
		{
			if ( m_bFreeArrayMemory )
				free( m_Data.m_pMemory );
			m_bFreeArrayMemory = false;
			m_nNumArrayElements = 0;
			m_Data.m_nMemory = 0;
			break;
		}
		default: 
			break;
	}
}

void KeyValues3::ResolveUnspecified()
{
	if ( GetSubType() == KV3_SUBTYPE_UNSPECIFIED )
	{
		switch ( GetType() )
		{
			case KV3_TYPE_NULL:
				m_SubType = KV3_SUBTYPE_NULL;
				break;
			case KV3_TYPE_BOOL:
				m_SubType = KV3_SUBTYPE_BOOL8;
				break;
			case KV3_TYPE_INT:
				m_SubType = KV3_SUBTYPE_INT64;
				break;
			case KV3_TYPE_UINT:
				m_SubType = KV3_SUBTYPE_UINT64;
				break;
			case KV3_TYPE_DOUBLE:
				m_SubType = KV3_SUBTYPE_FLOAT64;
				break;
			case KV3_TYPE_STRING:
				m_SubType = KV3_SUBTYPE_STRING;
				break;
			case KV3_TYPE_BINARY_BLOB:
				m_SubType = KV3_SUBTYPE_BINARY_BLOB;
				break;
			case KV3_TYPE_ARRAY:
				m_SubType = KV3_SUBTYPE_ARRAY;
				break;
			case KV3_TYPE_TABLE:
				m_SubType = KV3_SUBTYPE_TABLE;
				break;
			default:
				m_SubType = KV3_SUBTYPE_INVALID;
				break;
		}
	}
}

void KeyValues3::PrepareForType( KV3TypeEx_t type, KV3SubType_t subtype )
{
	if ( GetTypeEx() == type )
	{
		switch ( type )
		{
			case KV3_TYPEEX_STRING:
			case KV3_TYPEEX_BINARY_BLOB:
			case KV3_TYPEEX_BINARY_BLOB_EXTERN:
			case KV3_TYPEEX_ARRAY_FLOAT32:
			case KV3_TYPEEX_ARRAY_FLOAT64:
			case KV3_TYPEEX_ARRAY_INT16:
			case KV3_TYPEEX_ARRAY_INT32:
			case KV3_TYPEEX_ARRAY_UINT8_SHORT:
			case KV3_TYPEEX_ARRAY_INT16_SHORT:
			{
				Free();
				break;
			}
			default: 
				break;
		}
	}
	else
	{
		Free();
		m_TypeEx = type;
		Alloc();
	}

	m_SubType = subtype;
}

void KeyValues3::OnClearContext()
{ 
	Free( true ); 
	m_TypeEx = KV3_TYPEEX_NULL; 
	m_Data.m_nMemory = 0;
}

CKeyValues3Cluster* KeyValues3::GetCluster() const
{
	if ( m_bExternalStorage )
		return NULL;

	return GET_OUTER( CKeyValues3Cluster, m_Values[ m_nClusterElement ] );
}

CKeyValues3Context* KeyValues3::GetContext() const
{ 
	CKeyValues3Cluster* cluster = GetCluster();

	if ( cluster )
		return cluster->GetContext();
	else
		return NULL;
}

KV3MetaData_t* KeyValues3::GetMetaData( CKeyValues3Context** ppCtx ) const
{
	CKeyValues3Cluster* cluster = GetCluster();

	if ( cluster )
	{
		if ( ppCtx )
			*ppCtx = cluster->GetContext();

		return cluster->GetMetaData( m_nClusterElement );
	}
	else
	{
		if ( ppCtx )
			*ppCtx = NULL;

		return NULL;
	}
}

const char* KeyValues3::GetString( const char* defaultValue ) const
{
	switch ( GetTypeEx() )
	{
		case KV3_TYPEEX_STRING:
		case KV3_TYPEEX_STRING_EXTERN:
			return m_Data.m_pString;
		case KV3_TYPEEX_STRING_SHORT:
			return m_Data.m_szStringShort;
		default:
			return defaultValue;
	}
}

void KeyValues3::SetString( const char* pString, KV3SubType_t subtype )
{
	if ( !pString )
		pString = "";

	if ( strlen( pString ) < sizeof( m_Data.m_szStringShort ) )
	{
		PrepareForType( KV3_TYPEEX_STRING_SHORT, subtype );
		V_strncpy( m_Data.m_szStringShort, pString, sizeof( m_Data.m_szStringShort ) );
	}
	else
	{
		PrepareForType( KV3_TYPEEX_STRING, subtype );
		m_Data.m_pString = strdup( pString );
	}
}

void KeyValues3::SetStringExternal( const char* pString, KV3SubType_t subtype )
{
	if ( strlen( pString ) < sizeof( m_Data.m_szStringShort ) )
	{
		PrepareForType( KV3_TYPEEX_STRING_SHORT, subtype );
		V_strncpy( m_Data.m_szStringShort, pString, sizeof( m_Data.m_szStringShort ) );
	}
	else
	{
		PrepareForType( KV3_TYPEEX_STRING_EXTERN, subtype );
		m_Data.m_pString = pString;
	}
}

const byte* KeyValues3::GetBinaryBlob() const
{
	switch ( GetTypeEx() )
	{
		case KV3_TYPEEX_BINARY_BLOB:
			return m_Data.m_pBinaryBlob ? m_Data.m_pBinaryBlob->m_ubData : NULL;
		case KV3_TYPEEX_BINARY_BLOB_EXTERN:
			return m_Data.m_pBinaryBlob ? m_Data.m_pBinaryBlob->m_pubData : NULL;
		default:
			return NULL;
	}
}

int KeyValues3::GetBinaryBlobSize() const
{
	if ( GetType() != KV3_TYPE_BINARY_BLOB || !m_Data.m_pBinaryBlob )
		return 0;

	return ( int )m_Data.m_pBinaryBlob->m_nSize;
}

void KeyValues3::SetToBinaryBlob( const byte* blob, int size )
{
	PrepareForType( KV3_TYPEEX_BINARY_BLOB, KV3_SUBTYPE_BINARY_BLOB );

	if ( size > 0 )
	{
		m_Data.m_pBinaryBlob = (KV3BinaryBlob_t*)malloc( sizeof( size_t ) + size );
		m_Data.m_pBinaryBlob->m_nSize = size;
		memcpy( m_Data.m_pBinaryBlob->m_ubData, blob, size );
	}
	else
	{
		m_Data.m_pBinaryBlob = NULL;
	}
}

void KeyValues3::SetToBinaryBlobExternal( const byte* blob, int size, bool free_mem )
{
	PrepareForType( KV3_TYPEEX_BINARY_BLOB_EXTERN, KV3_SUBTYPE_BINARY_BLOB );

	if ( size > 0 )
	{
		m_Data.m_pBinaryBlob = (KV3BinaryBlob_t*)malloc( sizeof( KV3BinaryBlob_t ) );
		m_Data.m_pBinaryBlob->m_nSize = size;
		m_Data.m_pBinaryBlob->m_pubData = blob;
		m_Data.m_pBinaryBlob->m_bFreeMemory = free_mem;
	}
	else
	{
		m_Data.m_pBinaryBlob = NULL;
	}
}

Color KeyValues3::GetColor( const Color &defaultValue ) const
{
	int32 color[4];
	if ( ReadArrayInt32( 4, color ) )
	{
		return Color( color[0], color[1], color[2], color[3] );
	}
	else if ( ReadArrayInt32( 3, color ) )
	{
		return Color( color[0], color[1], color[2], 255 );
	}
	else
	{
		return defaultValue;
	}
}

void KeyValues3::SetColor( const Color &color )
{
	if ( color.a() == 255 )
		AllocArray<uint8>( 3, &color[0], KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_UINT8_SHORT, KV3_TYPEEX_INVALID, KV3_SUBTYPE_COLOR32, KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8 );
	else
		AllocArray<uint8>( 4, &color[0], KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_UINT8_SHORT, KV3_TYPEEX_INVALID, KV3_SUBTYPE_COLOR32, KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8 );
}

int KeyValues3::GetArrayElementCount() const
{
	if ( GetType() != KV3_TYPE_ARRAY )
		return 0;

	if ( GetTypeEx() == KV3_TYPEEX_ARRAY )
		return m_Data.m_pArray->Count();
	else
		return m_nNumArrayElements;
}

KeyValues3** KeyValues3::GetArrayBase()
{
	if ( GetTypeEx() != KV3_TYPEEX_ARRAY )
		return NULL;

	return m_Data.m_pArray->Base();
}

KeyValues3* KeyValues3::GetArrayElement( int elem )
{
	if ( GetTypeEx() != KV3_TYPEEX_ARRAY )
		return NULL;

	if ( elem < 0 || elem >= m_Data.m_pArray->Count() )
		return NULL;

	return m_Data.m_pArray->Element( elem );
}

KeyValues3* KeyValues3::InsertArrayElementBefore( int elem )
{
	if ( GetTypeEx() != KV3_TYPEEX_ARRAY )
		return NULL;

	return *m_Data.m_pArray->InsertBeforeGetPtr( elem, 1 );
}

KeyValues3* KeyValues3::AddArrayElementToTail()
{
	if ( GetTypeEx() != KV3_TYPEEX_ARRAY )
		return NULL;

	return *m_Data.m_pArray->InsertBeforeGetPtr( m_Data.m_pArray->Count(), 1 );
}

void KeyValues3::SetArrayElementCount( int count, KV3TypeEx_t type, KV3SubType_t subtype )
{
	if ( GetTypeEx() != KV3_TYPEEX_ARRAY )
		return;

	m_Data.m_pArray->SetCount( this, count, type, subtype );
}

void KeyValues3::RemoveArrayElements( int elem, int num )
{
	if ( GetTypeEx() != KV3_TYPEEX_ARRAY )
		return;

	m_Data.m_pArray->RemoveMultiple( elem, num );
}

void KeyValues3::NormalizeArray()
{
	switch ( GetTypeEx() )
	{
		case KV3_TYPEEX_ARRAY_FLOAT32:
		{
			NormalizeArray<float32>( KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT32, m_nNumArrayElements, m_Data.m_Array.m_f32, m_bFreeArrayMemory );
			break;
		}
		case KV3_TYPEEX_ARRAY_FLOAT64:
		{
			NormalizeArray<float64>( KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT64, m_nNumArrayElements, m_Data.m_Array.m_f64, m_bFreeArrayMemory );
			break;
		}
		case KV3_TYPEEX_ARRAY_INT16:
		{
			NormalizeArray<int16>( KV3_TYPEEX_INT, KV3_SUBTYPE_INT16, m_nNumArrayElements, m_Data.m_Array.m_i16, m_bFreeArrayMemory );
			break;
		}
		case KV3_TYPEEX_ARRAY_INT32:
		{
			NormalizeArray<int32>( KV3_TYPEEX_INT, KV3_SUBTYPE_INT32, m_nNumArrayElements, m_Data.m_Array.m_i32, m_bFreeArrayMemory );
			break;
		}
		case KV3_TYPEEX_ARRAY_UINT8_SHORT:
		{
			uint8 u8ArrayShort[8];
			memcpy( u8ArrayShort, m_Data.m_Array.m_u8Short, sizeof( u8ArrayShort ) );
			NormalizeArray<uint8>( KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8, m_nNumArrayElements, u8ArrayShort, false );
			break;
		}
		case KV3_TYPEEX_ARRAY_INT16_SHORT:
		{
			int16 i16ArrayShort[4];
			memcpy( i16ArrayShort, m_Data.m_Array.m_u8Short, sizeof( i16ArrayShort ) );
			NormalizeArray<int16>( KV3_TYPEEX_INT, KV3_SUBTYPE_INT16, m_nNumArrayElements, i16ArrayShort, false );
			break;
		}
		default: 
			break;
	}
}

bool KeyValues3::ReadArrayInt32( int dest_size, int32* data ) const
{
	int src_size = 0;

	if ( GetType() == KV3_TYPE_STRING )
	{
		CSplitString values( GetString(), " " );
		src_size = values.Count();
		int count = MIN( src_size, dest_size );
		for ( int i = 0; i < count; ++i )
			data[ i ] = V_StringToInt32( values[ i ], 0, NULL, NULL, PARSING_FLAG_SKIP_ASSERT );
	}
	else
	{
		switch ( GetTypeEx() )
		{
			case KV3_TYPEEX_ARRAY:
			{
				src_size = m_Data.m_pArray->Count();
				int count = MIN( src_size, dest_size );
				KeyValues3** arr = m_Data.m_pArray->Base();
				for ( int i = 0; i < count; ++i )
					data[ i ] = arr[ i ]->GetInt();
				break;
			}
			case KV3_TYPEEX_ARRAY_INT16:
			{
				src_size = m_nNumArrayElements;
				int count = MIN( src_size, dest_size );
				for ( int i = 0; i < count; ++i )
					data[ i ] = ( int32 )m_Data.m_Array.m_u8Short[ i ];
				break;
			}
			case KV3_TYPEEX_ARRAY_INT32:
			{
				src_size = m_nNumArrayElements;
				int count = MIN( src_size, dest_size );
				memcpy( data, m_Data.m_Array.m_i32, count * sizeof( int32 ) );
				break;
			}
			case KV3_TYPEEX_ARRAY_UINT8_SHORT:
			{
				src_size = m_nNumArrayElements;
				int count = MIN( src_size, dest_size );
				for ( int i = 0; i < count; ++i )
					data[ i ] = ( int32 )m_Data.m_Array.m_u8Short[ i ];
				break;
			}
			case KV3_TYPEEX_ARRAY_INT16_SHORT:
			{
				src_size = m_nNumArrayElements;
				int count = MIN( src_size, dest_size );
				for ( int i = 0; i < count; ++i )
					data[ i ] = ( int32 )m_Data.m_Array.m_u8Short[ i ];
				break;
			}
			default: 
				break;
		}
	}

	if ( src_size < dest_size )
		memset( &data[ src_size ], 0, ( dest_size - src_size ) * sizeof( int32 ) ); 

	return ( src_size == dest_size );
}

bool KeyValues3::ReadArrayFloat32( int dest_size, float32* data ) const
{
	int src_size = 0;

	if ( GetType() == KV3_TYPE_STRING )
	{
		CSplitString values( GetString(), " " );
		src_size = values.Count();
		int count = MIN( src_size, dest_size );
		for ( int i = 0; i < count; ++i )
			data[ i ] = ( float32 )V_StringToFloat64( values[ i ], 0, NULL, NULL, PARSING_FLAG_SKIP_ASSERT );
	}
	else
	{
		switch ( GetTypeEx() )
		{
			case KV3_TYPEEX_ARRAY:
			{
				src_size = m_Data.m_pArray->Count();
				int count = MIN( src_size, dest_size );
				KeyValues3** arr = m_Data.m_pArray->Base();
				for ( int i = 0; i < count; ++i )
					data[ i ] = arr[ i ]->GetFloat();
				break;
			}
			case KV3_TYPEEX_ARRAY_FLOAT32:
			{
				src_size = m_nNumArrayElements;
				int count = MIN( src_size, dest_size );
				memcpy( data, m_Data.m_Array.m_f32, count * sizeof( float32 ) );
				break;
			}
			case KV3_TYPEEX_ARRAY_FLOAT64:
			{
				src_size = m_nNumArrayElements;
				int count = MIN( src_size, dest_size );
				for ( int i = 0; i < count; ++i )
					data[ i ] = ( float32 )m_Data.m_Array.m_f64[ i ];
				break;
			}
			default: 
				break;
		}
	}

	if ( src_size < dest_size )
		memset( &data[ src_size ], 0, ( dest_size - src_size ) * sizeof( float32 ) ); 

	return ( src_size == dest_size );
}

int KeyValues3::GetMemberCount() const
{
	if ( GetType() != KV3_TYPE_TABLE )
		return 0;
	
	return m_Data.m_pTable->GetMemberCount();
}

KeyValues3* KeyValues3::GetMember( KV3MemberId_t id )
{
	if ( GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_Data.m_pTable->GetMemberCount() )
		return NULL;
	
	return m_Data.m_pTable->GetMember( id );
}

const char* KeyValues3::GetMemberName( KV3MemberId_t id ) const
{
	if ( GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_Data.m_pTable->GetMemberCount() )
		return NULL;
	
	return m_Data.m_pTable->GetMemberName( id );
}

CKV3MemberName KeyValues3::GetMemberNameEx( KV3MemberId_t id ) const
{
	if ( GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_Data.m_pTable->GetMemberCount() )
		return CKV3MemberName();

	return CKV3MemberName( m_Data.m_pTable->GetMemberHash( id ), m_Data.m_pTable->GetMemberName( id ) );
}

unsigned int KeyValues3::GetMemberHash( KV3MemberId_t id ) const
{
	if ( GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_Data.m_pTable->GetMemberCount() )
		return 0;
	
	return m_Data.m_pTable->GetMemberHash( id );
}

KeyValues3* KeyValues3::FindMember( const CKV3MemberName &name, KeyValues3* defaultValue )
{
	if ( GetType() != KV3_TYPE_TABLE )
		return defaultValue;

	KV3MemberId_t id = m_Data.m_pTable->FindMember( name );

	if ( id == KV3_INVALID_MEMBER )
		return defaultValue;

	return m_Data.m_pTable->GetMember( id );
}

KeyValues3* KeyValues3::FindOrCreateMember( const CKV3MemberName &name, bool *pCreated )
{
	if ( GetType() != KV3_TYPE_TABLE )
		PrepareForType( KV3_TYPEEX_TABLE, KV3_SUBTYPE_TABLE );

	KV3MemberId_t id = m_Data.m_pTable->FindMember( name );

	if ( id == KV3_INVALID_MEMBER )
	{
		if ( pCreated )
			*pCreated = true;

		id = m_Data.m_pTable->CreateMember( name );
	}
	else
	{
		if ( pCreated )
			*pCreated = false;
	}

	return m_Data.m_pTable->GetMember( id );
}

void KeyValues3::SetToEmptyTable()
{
	PrepareForType( KV3_TYPEEX_TABLE, KV3_SUBTYPE_TABLE );
	m_Data.m_pTable->RemoveAll();
}

bool KeyValues3::RemoveMember( KV3MemberId_t id )
{
	if ( GetType() != KV3_TYPE_TABLE || id < 0 || id >= m_Data.m_pTable->GetMemberCount() )
		return false;

	m_Data.m_pTable->RemoveMember( id );

	return true;
}

bool KeyValues3::RemoveMember( const KeyValues3* kv )
{
	if ( GetType() != KV3_TYPE_TABLE )
		return false;

	KV3MemberId_t id = m_Data.m_pTable->FindMember( kv );

	if ( id == KV3_INVALID_MEMBER )
		return false;

	m_Data.m_pTable->RemoveMember( id );

	return true;
}

bool KeyValues3::RemoveMember( const CKV3MemberName &name )
{
	if ( GetType() != KV3_TYPE_TABLE )
		return false;

	KV3MemberId_t id = m_Data.m_pTable->FindMember( name );

	if ( id == KV3_INVALID_MEMBER )
		return false;

	m_Data.m_pTable->RemoveMember( id );

	return true;
}

const char* KeyValues3::GetTypeAsString() const
{
	static const char* s_Types[] =
	{
		"invalid",
		"null",
		"bool",
		"int",
		"uint",
		"double",
		"string",
		"binary_blob",
		"array",
		"table",
		NULL
	};

	KV3Type_t type = GetType();

	if ( type < KV3_TYPE_COUNT )
		return s_Types[type];

	return "<unknown>";
}

const char* KeyValues3::GetSubTypeAsString() const
{
	static const char* s_SubTypes[] =
	{
		"invalid",
		"resource",
		"resource_name",
		"panorama",
		"soundevent",
		"subclass",
		"entity_name",
		"localize",
		"unspecified",
		"null",
		"binary_blob",
		"array",
		"table",
		"bool8",
		"char8",
		"uchar32",
		"int8",
		"uint8",
		"int16",
		"uint16",
		"int32",
		"uint32",
		"int64",
		"uint64",
		"float32",
		"float64",
		"string",
		"pointer",
		"color32",
		"vector",
		"vector2d",
		"vector4d",
		"rotation_vector",
		"quaternion",
		"qangle",
		"matrix3x4",
		"transform",
		"string_token",
		"ehandle",
		NULL
	};

	KV3SubType_t subtype = GetSubType();

	if ( subtype < KV3_SUBTYPE_COUNT )
		return s_SubTypes[subtype];

	return "<unknown>";
}

const char* KeyValues3::ToString( CBufferString& buff, uint flags ) const
{
	if ( ( flags & KV3_TO_STRING_DONT_CLEAR_BUFF ) != 0 )
		flags &= ~KV3_TO_STRING_DONT_APPEND_STRINGS;
	else
		buff.ToGrowable()->Clear();

	KV3Type_t type = GetType();

	switch ( type )
	{
		case KV3_TYPE_NULL:
		{
			return buff.ToGrowable()->Get();
		}
		case KV3_TYPE_BOOL:
		{
			const char* str = m_Data.m_Bool ? "true" : "false";

			if ( ( flags & KV3_TO_STRING_DONT_APPEND_STRINGS ) != 0 )
				return str;

			buff.Insert( buff.ToGrowable()->GetTotalNumber(), str );
			return buff.ToGrowable()->Get();
		}
		case KV3_TYPE_INT:
		{
			buff.AppendFormat( "%lld", m_Data.m_Int );
			return buff.ToGrowable()->Get();
		}
		case KV3_TYPE_UINT:
		{
			if ( GetSubType() == KV3_SUBTYPE_POINTER )
			{
				if ( ( flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS ) == 0 )
					buff.Insert( buff.ToGrowable()->GetTotalNumber(), "<pointer>" );

				if ( ( flags & KV3_TO_STRING_RETURN_NON_NUMERICS ) == 0 )
					return NULL;

				return buff.ToGrowable()->Get();
			}
			
			buff.AppendFormat( "%llu", m_Data.m_UInt );
			return buff.ToGrowable()->Get();
		}
		case KV3_TYPE_DOUBLE:
		{
			buff.AppendFormat( "%g", m_Data.m_Double );
			return buff.ToGrowable()->Get();
		}
		case KV3_TYPE_STRING:
		{
			const char* str = GetString();

			if ( ( flags & KV3_TO_STRING_DONT_APPEND_STRINGS ) != 0 )
				return str;

			buff.Insert( buff.ToGrowable()->GetTotalNumber(), str );
			return buff.ToGrowable()->Get();
		}
		case KV3_TYPE_BINARY_BLOB:
		{
			if ( ( flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS ) == 0 )
				buff.AppendFormat( "<binary blob: %u bytes>", GetBinaryBlobSize() );

			if ( ( flags & KV3_TO_STRING_RETURN_NON_NUMERICS ) == 0 )
				return NULL;

			return buff.ToGrowable()->Get();
		}
		case KV3_TYPE_ARRAY:
		{
			int elements = GetArrayElementCount();

			if ( elements > 0 && elements <= 4 )
			{
				switch ( GetTypeEx() )
				{
					case KV3_TYPEEX_ARRAY:
					{
						bool unprintable = false;
						CBufferStringGrowable<128> temp;

						CKeyValues3Array::Element_t* arr = m_Data.m_pArray->Base();
						for ( int i = 0; i < elements; ++i )
						{
							switch ( arr[i]->GetType() )
							{
								case KV3_TYPE_INT:
									temp.AppendFormat( "%lld", arr[i]->m_Data.m_Int );
									break;
								case KV3_TYPE_UINT:
									if ( arr[i]->GetSubType() == KV3_SUBTYPE_POINTER )
										unprintable = true;
									else
										temp.AppendFormat( "%llu", arr[i]->m_Data.m_UInt );
									break;
								case KV3_TYPE_DOUBLE:
									temp.AppendFormat( "%g", arr[i]->m_Data.m_Double );
									break;
								default:
									unprintable = true;
									break;
							}

							if ( unprintable )
								break;

							if ( i != elements - 1 ) temp.Insert( temp.ToGrowable()->GetTotalNumber(), " " );
						}

						if ( unprintable )
							break;

						buff.Insert( buff.ToGrowable()->GetTotalNumber(), temp.Get() );
						return buff.ToGrowable()->Get();
					}
					case KV3_TYPEEX_ARRAY_FLOAT32:
					{
						for ( int i = 0; i < elements; ++i )
						{
							buff.AppendFormat( "%g", m_Data.m_Array.m_f32[i] );
							if ( i != elements - 1 ) buff.Insert( buff.ToGrowable()->GetTotalNumber(), " " );
						}
						return buff.ToGrowable()->Get();
					}
					case KV3_TYPEEX_ARRAY_FLOAT64:
					{
						for ( int i = 0; i < elements; ++i )
						{
							buff.AppendFormat( "%g", m_Data.m_Array.m_f64[i] );
							if ( i != elements - 1 ) buff.Insert( buff.ToGrowable()->GetTotalNumber(), " " );
						}
						return buff.ToGrowable()->Get();
					}
					case KV3_TYPEEX_ARRAY_INT16:
					{
						for ( int i = 0; i < elements; ++i )
						{
							buff.AppendFormat( "%d", m_Data.m_Array.m_i16Short[i] );
							if ( i != elements - 1 ) buff.Insert( buff.ToGrowable()->GetTotalNumber(), " " );
						}
						return buff.ToGrowable()->Get();
					}
					case KV3_TYPEEX_ARRAY_INT32:
					{
						for ( int i = 0; i < elements; ++i )
						{
							buff.AppendFormat( "%d", m_Data.m_Array.m_i32[i] );
							if ( i != elements - 1 ) buff.Insert( buff.ToGrowable()->GetTotalNumber(), " " );
						}
						return buff.ToGrowable()->Get();
					}
					case KV3_TYPEEX_ARRAY_UINT8_SHORT:
					{
						for ( int i = 0; i < elements; ++i )
						{
							buff.AppendFormat( "%u", m_Data.m_Array.m_u8Short[i] );
							if ( i != elements - 1 ) buff.Insert( buff.ToGrowable()->GetTotalNumber(), " " );
						}
						return buff.ToGrowable()->Get();
					}
					case KV3_TYPEEX_ARRAY_INT16_SHORT:
					{
						for ( int i = 0; i < elements; ++i )
						{
							buff.AppendFormat( "%d", m_Data.m_Array.m_i16Short[i] );
							if ( i != elements - 1 ) buff.Insert( buff.ToGrowable()->GetTotalNumber(), " " );
						}
						return buff.ToGrowable()->Get();
					}
					default:
						break;
				}
			}

			if ( ( flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS ) == 0 )
				buff.AppendFormat( "<array: %u elements>", elements );

			if ( ( flags & KV3_TO_STRING_RETURN_NON_NUMERICS ) == 0 )
				return NULL;

			return buff.ToGrowable()->Get();
		}
		case KV3_TYPE_TABLE:
		{
			if ( ( flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS ) == 0 )
				buff.AppendFormat( "<table: %u members>", GetMemberCount() );

			if ( ( flags & KV3_TO_STRING_RETURN_NON_NUMERICS ) == 0 )
				return NULL;

			return buff.ToGrowable()->Get();
		}
		default: 
		{
			if ( ( flags & KV3_TO_STRING_APPEND_ONLY_NUMERICS ) == 0 )
				buff.AppendFormat( "<unknown KV3 basic type '%s' (%d)>", GetTypeAsString(), type );

			if ( ( flags & KV3_TO_STRING_RETURN_NON_NUMERICS ) == 0 )
				return NULL;

			return buff.ToGrowable()->Get();
		}
	}
}

void KeyValues3::CopyFrom( const KeyValues3* pSrc )
{
	SetToNull();

	CKeyValues3Context* context;
	KV3MetaData_t* pDestMetaData = GetMetaData( &context );

	if ( pDestMetaData )
	{
		KV3MetaData_t* pSrcMetaData = pSrc->GetMetaData();

		if ( pSrcMetaData )
			context->CopyMetaData( pDestMetaData, pSrcMetaData );
		else
			pDestMetaData->Clear();
	}

	KV3SubType_t eSrcSubType = pSrc->GetSubType();

	switch ( pSrc->GetType() )
	{
		case KV3_TYPE_BOOL:
			SetBool( pSrc->m_Data.m_Bool );
			break;
		case KV3_TYPE_INT:
			SetValue<int64>( pSrc->m_Data.m_Int, KV3_TYPEEX_INT, eSrcSubType );
			break;
		case KV3_TYPE_UINT:
			SetValue<uint64>( pSrc->m_Data.m_UInt, KV3_TYPEEX_UINT, eSrcSubType );
			break;
		case KV3_TYPE_DOUBLE:
			SetValue<float64>( pSrc->m_Data.m_Double, KV3_TYPEEX_DOUBLE, eSrcSubType );
			break;
		case KV3_TYPE_STRING:
			SetString( pSrc->GetString(), eSrcSubType );
			break;
		case KV3_TYPE_BINARY_BLOB:
			SetToBinaryBlob( pSrc->GetBinaryBlob(), pSrc->GetBinaryBlobSize() );
			break;
		case KV3_TYPE_ARRAY:
		{
			switch ( pSrc->GetTypeEx() )
			{
				case KV3_TYPEEX_ARRAY:
				{
					PrepareForType( KV3_TYPEEX_ARRAY, KV3_SUBTYPE_ARRAY );
					m_Data.m_pArray->CopyFrom( this, pSrc->m_Data.m_pArray );
					break;
				}
				case KV3_TYPEEX_ARRAY_FLOAT32:
					AllocArray<float32>( pSrc->m_nNumArrayElements, pSrc->m_Data.m_Array.m_f32, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_INVALID, KV3_TYPEEX_ARRAY_FLOAT32, eSrcSubType, KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT32 );
					break;
				case KV3_TYPEEX_ARRAY_FLOAT64:
					AllocArray<float64>( pSrc->m_nNumArrayElements, pSrc->m_Data.m_Array.m_f64, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_INVALID, KV3_TYPEEX_ARRAY_FLOAT64, eSrcSubType, KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT64 );
					break;
				case KV3_TYPEEX_ARRAY_INT16:
					AllocArray<int16>( pSrc->m_nNumArrayElements, pSrc->m_Data.m_Array.m_i16, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_INT16_SHORT, KV3_TYPEEX_ARRAY_INT16, eSrcSubType, KV3_TYPEEX_INT, KV3_SUBTYPE_INT16 );
					break;
				case KV3_TYPEEX_ARRAY_INT32:
					AllocArray<int32>( pSrc->m_nNumArrayElements, pSrc->m_Data.m_Array.m_i32, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_INVALID, KV3_TYPEEX_ARRAY_INT32, eSrcSubType, KV3_TYPEEX_INT, KV3_SUBTYPE_INT32 );
					break;
				case KV3_TYPEEX_ARRAY_UINT8_SHORT:
					AllocArray<uint8>( pSrc->m_nNumArrayElements, pSrc->m_Data.m_Array.m_u8Short, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_UINT8_SHORT, KV3_TYPEEX_INVALID, eSrcSubType, KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8 );
					break;
				case KV3_TYPEEX_ARRAY_INT16_SHORT:
					AllocArray<int16>( pSrc->m_nNumArrayElements, pSrc->m_Data.m_Array.m_i16Short, KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_ARRAY_INT16_SHORT, KV3_TYPEEX_ARRAY_INT16, eSrcSubType, KV3_TYPEEX_INT, KV3_SUBTYPE_INT16 );
					break;
				default:
					break;
			}
			break;
		}
		case KV3_TYPE_TABLE:
		{
			PrepareForType( KV3_TYPEEX_TABLE, KV3_SUBTYPE_TABLE );
			m_Data.m_pTable->CopyFrom( pSrc->m_Data.m_pTable );
			break;
		}
		default:
			break;
	}

	m_SubType = eSrcSubType;
	m_nFlags = pSrc->m_nFlags;
}

KeyValues3& KeyValues3::operator=( const KeyValues3& src )
{
	if ( this == &src )
		return *this;

	CopyFrom( &src );

	return *this;
}

CKeyValues3Array::CKeyValues3Array( int cluster_elem, int alloc_size ) :
	m_nClusterElement( cluster_elem ),
	m_nAllocatedChunks( alloc_size ),
	m_nCount( 0 ),
	m_nInitialSize( MIN( alloc_size, 255 ) ),
	m_bIsDynamicallySized( false ),
	m_unk001( false ),
	m_unk002( false ),
	m_pDynamicElements( nullptr )
{
}

CKeyValues3ArrayCluster* CKeyValues3Array::GetCluster() const
{
	if ( m_nClusterElement == -1 )
		return NULL;

	return GET_OUTER( CKeyValues3ArrayCluster, m_Values[ m_nClusterElement ] );
}

CKeyValues3Context* CKeyValues3Array::GetContext() const
{ 
	CKeyValues3ArrayCluster* cluster = GetCluster();

	if ( cluster )
		return cluster->GetContext();
	else
		return NULL;
}

KeyValues3* CKeyValues3Array::Element( int i )
{
	Assert( 0 <= i && i < m_nCount );

	return Base()[i];
}

void CKeyValues3Array::SetCount( KeyValues3 *kv, int count, KV3TypeEx_t type, KV3SubType_t subtype )
{
	int nOldSize = m_nCount;

	CKeyValues3Context* context = kv->GetContext();

	for ( int i = count; i < nOldSize; ++i )
	{
		Element_t pElement = m_StaticElements[i];

		if ( context && kv->m_bExternalStorage )
			context->FreeKV( pElement );
		else
		{
			pElement->Free( true );
			free( pElement );
		}
	}

	Element_t *pNew = &m_StaticElements[0];

	if ( count > MAX( KV3_ARRAY_MAX_FIXED_MEMBERS, m_nAllocatedChunks ) )
	{
		int new_byte_size = TotalSizeOfData( count );
		pNew = (Element_t *)( m_bIsDynamicallySized ? realloc( m_pDynamicElements, new_byte_size ) : malloc( new_byte_size ) );

		memmove( pNew, Base(), count * sizeof(Element_t) );

		m_pDynamicElements = pNew;

		if ( count > m_nAllocatedChunks )
		{
			m_nAllocatedChunks = count;
		}

		m_bIsDynamicallySized = true;
	}

	for ( int i = nOldSize; i < count; ++i )
	{
		if(context)
		{
			pNew[i] = context->AllocKV( type, subtype );
		}
		else
			pNew[i] = new KeyValues3( type, subtype );
	}

	m_nCount = count;
}

CKeyValues3Array::Element_t* CKeyValues3Array::InsertBeforeGetPtr( int elem, int num )
{
	Element_t *kv = Base();

	CKeyValues3Context* context = GetContext();

	for ( int i = 0; i < num; ++i )
	{
		if(context)
		{
			kv[elem + i] = context->AllocKV();
		}
		else
			kv[elem + i] = new KeyValues3;
	}

	return kv;
}

void CKeyValues3Array::CopyFrom( KeyValues3 *kv, const CKeyValues3Array* pSrc )
{
	Element_t* base = Base();
	Element_t const* pSrcKV = pSrc->Base();

	int nNewSize = pSrc->Count();

	SetCount( kv, nNewSize );

	for ( int i = 0; i < nNewSize; ++i )
		*base[i] = *pSrcKV[i];
}

void CKeyValues3Array::RemoveMultiple( int elem, int num )
{
	CKeyValues3Context* context = GetContext();
	Element_t *kv = Base();

	for ( int i = 0; i <= num; ++i )
	{
		auto &Element = kv[ elem + i ];

		if ( context )
			context->FreeKV( Element );
		else
		{
			Element->Free( true );
			free( Element );
		}
	}
}

void CKeyValues3Array::Purge( bool bClearingContext )
{ 
	CKeyValues3Context* context = GetContext();
	Element_t *kv = Base();

	for ( int i = 0; i < m_nCount; i++ )
	{
		if ( context )
		{
			if ( !bClearingContext )
				context->FreeKV( kv[ i ] );
		}
		else
		{
			kv[ i ]->Free( true );
			free( kv[ i ] );
		}
	}

	if ( m_bIsDynamicallySized )
		free( m_pDynamicElements );

	m_nCount = 0;
}

CKeyValues3Table::CKeyValues3Table( int cluster_elem, int alloc_size ) :
	m_nClusterElement( cluster_elem ),
	m_nAllocatedChunks( alloc_size ),
	m_pFastSearch( nullptr ),
	m_nCount( 0 ),
	m_nInitialSize( MIN( alloc_size, 255 ) ),
	m_bIsDynamicallySized( false ),
	m_unk001( false ),
	m_unk002( false ),
	m_pDynamicBuffer( nullptr )
{
}

CKeyValues3TableCluster* CKeyValues3Table::GetCluster() const
{
	if ( m_nClusterElement == -1 )
		return NULL;

	return GET_OUTER( CKeyValues3TableCluster, m_Values[ m_nClusterElement ] );
}

CKeyValues3Context* CKeyValues3Table::GetContext() const
{ 
	CKeyValues3TableCluster* cluster = GetCluster();

	if ( cluster )
		return cluster->GetContext();
	else
		return NULL;
}

KeyValues3* CKeyValues3Table::GetMember( KV3MemberId_t id )
{
	Assert( 0 <= id && id < m_nCount );

	return MembersBase()[id];
}

const CKeyValues3Table::Name_t CKeyValues3Table::GetMemberName( KV3MemberId_t id ) const
{
	Assert( 0 <= id && id < m_nCount );

	return NamesBase()[id];
}

const CKeyValues3Table::Hash_t CKeyValues3Table::GetMemberHash( KV3MemberId_t id ) const
{
	Assert( 0 <= id && id < m_nCount );

	return HashesBase()[id];
}

void CKeyValues3Table::EnableFastSearch()
{
	if ( m_pFastSearch )
		m_pFastSearch->m_member_ids.RemoveAll();
	else
		m_pFastSearch = new kv3tablefastsearch_t;

	const Hash_t* pHashes = HashesBase();

	for ( int i = 0; i < m_nCount; ++i )
	{
		m_pFastSearch->m_member_ids.Insert( pHashes[i], i );
	}

	m_pFastSearch->m_ignore = false;
	m_pFastSearch->m_ignores_counter = 0;
}

void CKeyValues3Table::EnsureMemberCapacity( int num, bool force, bool dont_move )
{
	if ( num <= m_nAllocatedChunks )
		return;

	int nNewAllocatedChunks = m_nAllocatedChunks;

	if ( num > KV3_MAX_CHUNKS)
	{
		Plat_FatalErrorFunc( "%s member count overflow (%u)\n", __FUNCTION__, num );
		DebuggerBreak();
	}

	if ( force )
	{
		nNewAllocatedChunks = num;
	}
	else
	{
		nNewAllocatedChunks = MAX( KV3_MIN_CHUNKS, nNewAllocatedChunks );

		while ( nNewAllocatedChunks < num )
		{
			if ( nNewAllocatedChunks < KV3_MAX_CHUNKS / 2 )
				nNewAllocatedChunks = nNewAllocatedChunks * 2;
			else
			{
				nNewAllocatedChunks = KV3_MAX_CHUNKS;
				break;
			}
		}
	}

	const int new_byte_size = TotalSizeOfData( nNewAllocatedChunks );
	void* new_base = m_bIsDynamicallySized ? realloc( m_pDynamicBuffer, new_byte_size ) : malloc( new_byte_size );

	if(m_nCount == 0)
		dont_move = true;

	if ( !dont_move )
	{
		if ( m_bIsDynamicallySized )
		{
			memmove( (uint8 *)new_base + OffsetToIsExternalNameBase( nNewAllocatedChunks ), IsExternalNameBase(), m_nCount * sizeof(IsExternalName_t) );
			memmove( (uint8 *)new_base + OffsetToNamesBase( nNewAllocatedChunks ), NamesBase(), m_nCount * sizeof(Name_t) );
			memmove( (uint8 *)new_base + OffsetToMembersBase( nNewAllocatedChunks ), MembersBase(), m_nCount * sizeof(Member_t) );
		}
		else
		{
			memmove( (uint8 *)new_base + OffsetToHashesBase( nNewAllocatedChunks ), HashesBase(), m_nCount * sizeof(Hash_t) );
			memmove( (uint8 *)new_base + OffsetToMembersBase( nNewAllocatedChunks ), MembersBase(), m_nCount * sizeof(Member_t) );
			memmove( (uint8 *)new_base + OffsetToNamesBase( nNewAllocatedChunks ), NamesBase(), m_nCount * sizeof(Name_t) );
			memmove( (uint8 *)new_base + OffsetToIsExternalNameBase( nNewAllocatedChunks ), IsExternalNameBase(), m_nCount * sizeof(IsExternalName_t) );
		}
	}

	m_pDynamicBuffer = new_base;
	m_nAllocatedChunks = nNewAllocatedChunks;
	m_bIsDynamicallySized = true;
}

KV3MemberId_t CKeyValues3Table::FindMember( const KeyValues3* kv ) const
{
	const Member_t* pMembers = MembersBase();

	for ( int i = 0; i < m_nCount; ++i )
	{
		if ( pMembers[i] == kv )
			return i;
	}

	return KV3_INVALID_MEMBER;
}

KV3MemberId_t CKeyValues3Table::FindMember( const CKV3MemberName &name )
{
	bool bFastSearch = false;

	if ( m_pFastSearch )
	{
		if ( m_pFastSearch->m_ignore )
		{
			if ( ++m_pFastSearch->m_ignores_counter > 4 )
			{
				EnableFastSearch();
				bFastSearch = true;
			}
		}
		else
		{
			bFastSearch = true;
		}
	}

	if ( bFastSearch )
	{
		UtlHashHandle_t h = m_pFastSearch->m_member_ids.Find( name.GetHashCode() );

		if ( h != m_pFastSearch->m_member_ids.InvalidHandle() )
			return m_pFastSearch->m_member_ids[ h ];
	}
	else
	{
		const Hash_t* pHashes = HashesBase();

		for ( int i = 0; i < m_nCount; ++i )
		{
			if ( pHashes[i] == name.GetHashCode() )
				return i;
		}
	}

	return KV3_INVALID_MEMBER;
}

KV3MemberId_t CKeyValues3Table::CreateMember( const CKV3MemberName &name )
{
	if ( GetMemberCount() >= 128 && !m_pFastSearch )
		EnableFastSearch();

	KV3MemberId_t memberId = m_nCount;

	int nNewSize = m_nCount + 1;

	if ( nNewSize > KV3_TABLE_MAX_FIXED_MEMBERS )
		EnsureMemberCapacity( nNewSize );

	CKeyValues3Context* context = GetContext();

	Hash_t* pHashes = HashesBase();
	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();

	pHashes[memberId] = name.GetHashCode();

	if ( context )
	{
		pMembers[memberId] = context->AllocKV();
		pNames[memberId] = context->AllocString( name.GetString() );
	}
	else
	{
		pMembers[memberId] = new KeyValues3;
		pNames[memberId] = strdup( name.GetString() );
	}

	if ( m_pFastSearch && !m_pFastSearch->m_ignore )
		m_pFastSearch->m_member_ids.Insert( name.GetHashCode(), memberId );

	m_nCount = nNewSize;

	return memberId;
}

void CKeyValues3Table::CopyFrom( const CKeyValues3Table* pSrc )
{
	int nNewSize = m_nCount;

	RemoveAll( nNewSize );

	CKeyValues3Context* context = GetContext();

	Hash_t* pHashes = HashesBase();
	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();

	const Hash_t* pCopyHashes = pSrc->HashesBase();
	const Member_t* pCopyMembers = pSrc->MembersBase();
	const Name_t* pCopyNames = pSrc->NamesBase();

	for ( int i = 0; i < nNewSize; ++i )
	{
		pHashes[i] = pCopyHashes[i];

		if ( context )
		{
			pMembers[i] = context->AllocKV();
			pNames[i] = context->AllocString( pCopyNames[i] );
		}
		else
		{
			pMembers[i] = new KeyValues3;
			pNames[i] = strdup( pCopyNames[i] );
		}

		*pCopyMembers[i] = *pCopyMembers[i];
	}

	if ( nNewSize >= 128 )
		EnableFastSearch();
}

void CKeyValues3Table::RemoveMember( KV3MemberId_t id )
{
	CKeyValues3Context* context = GetContext();

	Hash_t* pHashes = HashesBase();
	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();
	IsExternalName_t* pIsExternalNames = IsExternalNameBase();

	if ( context ) 
	{
		context->FreeKV( pMembers[ id ] );
	}
	else
	{
		pMembers[ id ]->Free( true );
		free( pMembers[ id ] );
		free( (void*)pNames[ id ] );
	}

	KV3MemberId_t nShiftFrom = id + 1;

	if ( nShiftFrom <= m_nCount )
	{
		int nHighElements = m_nCount - nShiftFrom;

		memmove( &pHashes[id], &pHashes[nShiftFrom], nHighElements * sizeof(Hash_t) );
		memmove( &pMembers[id], &pMembers[nShiftFrom], nHighElements * sizeof(Member_t) );
		memmove( &pNames[id], &pNames[nShiftFrom], nHighElements * sizeof(Name_t) );
		memmove( &pIsExternalNames[id], &pIsExternalNames[nShiftFrom], nHighElements * sizeof(IsExternalName_t) );
	}


	if ( m_pFastSearch )
	{
		m_pFastSearch->m_ignore = true;
		m_pFastSearch->m_ignores_counter = 1;
	}

	m_nCount--;
}

void CKeyValues3Table::RemoveAll( int nAllocSize )
{
	CKeyValues3Context* context = GetContext();

	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();

	for ( int i = 0; i < m_nCount; ++i )
	{
		if ( context )
		{
			context->FreeKV( pMembers[i] );
		}
		else
		{
			pMembers[i]->Free( true );
			free( pMembers[i] );
			free( (void*)pNames[i] );
		}
	}

	if ( nAllocSize > KV3_TABLE_MAX_FIXED_MEMBERS )
	{
		EnsureMemberCapacity( nAllocSize, true, true );
	}
	else if ( m_bIsDynamicallySized )
	{
		free( m_pDynamicBuffer );
	}
	m_nCount = nAllocSize;

	if ( m_pFastSearch )
	{
		if ( nAllocSize >= 128 )
		{
			m_pFastSearch->Clear();
		}
		else
		{
			delete m_pFastSearch;
			m_pFastSearch = NULL;
		}
	}
}

void CKeyValues3Table::Purge( bool bClearingContext )
{
	CKeyValues3Context* context = GetContext();

	Member_t* pMembers = MembersBase();
	Name_t* pNames = NamesBase();

	for ( int i = 0; i < m_nCount; ++i )
	{
		if ( context )
		{
			if ( !bClearingContext )
				context->FreeKV( pMembers[i] );
		}
		else
		{
			pMembers[i]->Free( true );
			free( pMembers[i] );
			free( (void*)pNames[i] );
		}
	}

	if ( m_bIsDynamicallySized )
	{
		free( m_pDynamicBuffer );
	}
	m_nAllocatedChunks = 0;
	m_bIsDynamicallySized = false;
	m_nCount = 0;

	if ( m_pFastSearch )
		delete m_pFastSearch;
	m_pFastSearch = nullptr;
}

CKeyValues3ContextBase::CKeyValues3ContextBase( CKeyValues3Context* context ) : 	
	m_pContext( context ),
	m_KV3BaseCluster( context ),
	m_pParsingErrorListener( nullptr ),
	m_bMetaDataEnabled( false ),
	m_bFormatConverted( false ),
	m_bRootAvailabe( false )
{
}

void CKeyValues3ContextBase::Clear()
{
	m_BinaryData.Clear();
	m_KV3BaseCluster.Clear();
	m_Symbols.RemoveAll();
	
	m_bFormatConverted = false;
}

void CKeyValues3ContextBase::Purge()
{
	m_BinaryData.Purge();
	m_KV3BaseCluster.Purge();
	m_Symbols.Purge();

	m_bFormatConverted = false;
}

CKeyValues3Context::CKeyValues3Context( bool bNoRoot ) : BaseClass( this ), pad{}
{
	if ( bNoRoot )
	{
		m_bRootAvailabe =  false;
	}
	else
	{
		m_bRootAvailabe = true;
		m_KV3BaseCluster.Alloc();
	}

	m_bMetaDataEnabled = false;
	m_bFormatConverted = false;
}

void CKeyValues3Context::Clear()
{
	BaseClass::Clear();

	ClearClusterNodeChain( &m_KV3PartialClusters );
	ClearClusterNodeChain( &m_KV3FullClusters );
	MoveToPartial( &m_KV3FullClusters, &m_KV3PartialClusters );

	ClearClusterNodeChain( &m_PartialArrayClusters );
	ClearClusterNodeChain( &m_FullArrayClusters );
	MoveToPartial( &m_FullArrayClusters, &m_PartialArrayClusters );
	m_RawArrayEntries.Clear();

	ClearClusterNodeChain( &m_PartialTableClusters );
	ClearClusterNodeChain( &m_FullTableClusters );
	MoveToPartial( &m_FullTableClusters, &m_PartialTableClusters );
	m_RawTableEntries.Clear();

	if ( m_bRootAvailabe )
		m_KV3BaseCluster.Alloc();
}

void CKeyValues3Context::Purge()
{
	BaseClass::Purge();

	PurgeClusterNodeChain( &m_KV3PartialClusters );
	PurgeClusterNodeChain( &m_KV3FullClusters );
	m_KV3PartialClusters.AddToChain( &m_KV3BaseCluster );

	PurgeClusterNodeChain( &m_PartialArrayClusters );
	PurgeClusterNodeChain( &m_FullArrayClusters );
	m_RawArrayEntries.Purge();

	PurgeClusterNodeChain( &m_PartialTableClusters );
	PurgeClusterNodeChain( &m_FullTableClusters );
	m_RawTableEntries.Purge();

	if ( m_bRootAvailabe )
		m_KV3BaseCluster.Alloc();
}

KeyValues3* CKeyValues3Context::Root()
{
	if ( !m_bRootAvailabe )
	{
		Plat_FatalErrorFunc( "FATAL: %s called on a pool context (no root available)\n", __FUNCTION__ );
		DebuggerBreak();
	}

	return &m_KV3BaseCluster.Head()->m_Value;
}

const char* CKeyValues3Context::AllocString( const char* pString )
{
	return m_Symbols.AddString( pString ).String();
}

void CKeyValues3Context::EnableMetaData( bool bEnable )
{
	if ( bEnable != m_bMetaDataEnabled )
	{
		m_KV3BaseCluster.EnableMetaData( bEnable );

		m_bMetaDataEnabled = bEnable;
	}
}

void CKeyValues3Context::CopyMetaData( KV3MetaData_t* pDest, const KV3MetaData_t* pSrc )
{
	pDest->m_nLine = pSrc->m_nLine;
	pDest->m_nColumn = pSrc->m_nColumn;
	pDest->m_nFlags = pSrc->m_nFlags;
	pDest->m_sName = m_Symbols.AddString( pSrc->m_sName.String() );

	pDest->m_Comments.Purge();
	pDest->m_Comments.EnsureCapacity( pSrc->m_Comments.Count() );

	FOR_EACH_MAP_FAST( pSrc->m_Comments, iter )
	{
		pDest->m_Comments.Insert( pSrc->m_Comments.Key( iter ), pSrc->m_Comments.Element( iter ) );
	}
}

KeyValues3* CKeyValues3Context::AllocKV( KV3TypeEx_t type, KV3SubType_t subtype )
{
	return Alloc( &m_KV3PartialClusters, &m_KV3FullClusters, CKeyValues3Cluster::SIZE, type, subtype );
}

CKeyValues3Array *CKeyValues3Context::AllocArray( int initial_size )
{
	int needed_byte_size = MAX( CKeyValues3Array::TotalSizeOf( initial_size ), 32 );

	if(m_RawArrayEntries.IsFull() || needed_byte_size > m_RawArrayEntries.FreeBytes())
	{
		if(initial_size <= CKeyValues3Array::DATA_SIZE)
			return Alloc( &m_PartialArrayClusters, &m_FullArrayClusters );
		else
			return nullptr;
	}
	
	return m_RawArrayEntries.Alloc( initial_size );
}

CKeyValues3Table *CKeyValues3Context::AllocTable( int initial_size )
{
	int needed_byte_size = MAX( CKeyValues3Table::TotalSizeOf( initial_size ), 32 );

	if(m_RawArrayEntries.IsFull() || needed_byte_size > m_RawArrayEntries.FreeBytes())
	{
		if(initial_size <= CKeyValues3Array::DATA_SIZE)
			return Alloc( &m_PartialTableClusters, &m_FullTableClusters );
		else
			return nullptr;
	}

	return m_RawTableEntries.Alloc( initial_size );
}

void CKeyValues3Context::FreeKV( KeyValues3* kv )
{
	CKeyValues3Context* context;
	KV3MetaData_t* metadata = kv->GetMetaData( &context );

	if ( metadata )
		metadata->Clear();

	// Free<KeyValues3, CKeyValues3Cluster>( kv, &m_KV3BaseCluster, m_pKV3FreeCluster );
}

#include "tier0/memdbgoff.h"