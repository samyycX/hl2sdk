#ifndef KEYVALUES3_H
#define KEYVALUES3_H

#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "tier0/dbg.h"
#include "tier1/bufferstring.h"
#include "tier1/generichash.h"
#include "tier1/strtools.h"
#include "tier1/utlbuffer.h"
#include "tier1/utlhashtable.h"
#include "tier1/utlmap.h"
#include "tier1/utlstring.h"
#include "tier1/utlstringtoken.h"
#include "tier1/utlsymbollarge.h"
#include "mathlib/vector4d.h"
#include "Color.h"
#include "bitvec.h"
#include "entityhandle.h"

#include <type_traits>

#include "tier0/memdbgon.h"

class KeyValues3;
class CKeyValues3Array;
class CKeyValues3Table;
class CKeyValues3Context;
struct KV1ToKV3Translation_t;
struct KV3ToKV1Translation_t;

/* 
	KeyValues3 is a data storage format. See https://developer.valvesoftware.com/wiki/KeyValues3
	Supports various specific data types targeted at the Source2.
	Each specific type corresponds to one of the basic types.

	There are 2 ways to create KeyValues3:

	1. Via CKeyValues3Context:
	- KV's, arrays and tables are stored in fixed memory blocks (clusters) and therefore memory is allocated only when clusters are created.
	- Supports metadata and some other things.

	2. Directly through the constructor.
*/

struct KV3ID_t
{
	const char* m_name;
	uint64		m_data1;
	uint64		m_data2;
};

// encodings
const KV3ID_t g_KV3Encoding_Text 		= { "text", 0x41C58A33E21C7F3Cull, 0xDAA323A6DA77799ull };
const KV3ID_t g_KV3Encoding_Binary 		= { "binary", 0x40C1F7D81B860500ull, 0x14E76782A47582ADull };
const KV3ID_t g_KV3Encoding_BinaryLZ4 	= { "binary_lz4", 0x4F5C63A16847348Aull, 0x19B1D96F805397A1ull };
const KV3ID_t g_KV3Encoding_BinaryZSTD 	= { "binary_zstd", 0x4305FEF06F620A00ull, 0x29DBB14623045FA3ull };
const KV3ID_t g_KV3Encoding_BinaryBC 	= { "binary_bc", 0x4F6C95BC95791A46ull, 0xD2DFB7A1BC050BA7ull };
const KV3ID_t g_KV3Encoding_BinaryAuto 	= { "binary_auto", 0x45836B856EB109E6ull, 0x8C06046E3A7012A3ull };

// formats
const KV3ID_t g_KV3Format_Generic = { "generic", 0x469806E97412167Cull, 0xE73790B53EE6F2AFull };

enum KV1TextEscapeBehavior_t
{
	KV1TEXT_ESC_BEHAVIOR_UNK1 = 0,
	KV1TEXT_ESC_BEHAVIOR_UNK2 = 1,
};

enum KV3SaveTextFlags_t
{
	KV3_SAVE_TEXT_NONE = 0,
	KV3_SAVE_TEXT_TAGGED = (1 << 0), // adds subtype name before value
};

PLATFORM_OVERLOAD void DebugPrintKV3( const KeyValues3* kv );

// When using some LoadKV3/SaveKV3 functions, KV3ID_t structures must be filled in, which specify the format or encoding of the data.

PLATFORM_OVERLOAD bool LoadKV3( CKeyValues3Context* context, CUtlString* error, CUtlBuffer* input, const KV3ID_t& format, const char* kv_name );
PLATFORM_OVERLOAD bool LoadKV3( KeyValues3* kv, CUtlString* error, CUtlBuffer* input, const KV3ID_t& format, const char* kv_name );
PLATFORM_OVERLOAD bool LoadKV3( KeyValues3* kv, CUtlString* error, const char* input, const KV3ID_t& format, const char* kv_name );
PLATFORM_OVERLOAD bool LoadKV3FromFile( CKeyValues3Context* context, CUtlString* error, const char* filename, const char* path, const KV3ID_t& format );
PLATFORM_OVERLOAD bool LoadKV3FromFile( KeyValues3* kv, CUtlString* error, const char* filename, const char* path, const KV3ID_t& format );
PLATFORM_OVERLOAD bool LoadKV3FromJSON( KeyValues3* kv, CUtlString* error, const char* input, const char* kv_name );
PLATFORM_OVERLOAD bool LoadKV3FromJSONFile( KeyValues3* kv, CUtlString* error, const char* path, const char* filename );
PLATFORM_OVERLOAD bool LoadKV3FromKV1File( KeyValues3* kv, CUtlString* error, const char* path, const char* filename, KV1TextEscapeBehavior_t esc_behavior );
PLATFORM_OVERLOAD bool LoadKV3FromKV1Text( KeyValues3* kv, CUtlString* error, const char* input, KV1TextEscapeBehavior_t esc_behavior, const char* kv_name, bool unk );
PLATFORM_OVERLOAD bool LoadKV3FromKV1Text_Translated( KeyValues3* kv, CUtlString* error, const char* input, KV1TextEscapeBehavior_t esc_behavior, const KV1ToKV3Translation_t* translation, int unk1, const char* kv_name, bool unk2 );
PLATFORM_OVERLOAD bool LoadKV3FromKV3OrKV1( KeyValues3* kv, CUtlString* error, CUtlBuffer* input, const KV3ID_t& format, const char* kv_name );
PLATFORM_OVERLOAD bool LoadKV3FromOldSchemaText( KeyValues3* kv, CUtlString* error, CUtlBuffer* input, const KV3ID_t& format, const char* kv_name );
PLATFORM_OVERLOAD bool LoadKV3Text_NoHeader( KeyValues3* kv, CUtlString* error, const char* input, const KV3ID_t& format, const char* kv_name );

PLATFORM_OVERLOAD bool SaveKV3( const KV3ID_t& encoding, const KV3ID_t& format, const KeyValues3* kv, CUtlString* error, CUtlBuffer* output, uint flags = KV3_SAVE_TEXT_NONE );
PLATFORM_OVERLOAD bool SaveKV3AsJSON( const KeyValues3* kv, CUtlString* error, CUtlBuffer* output );
PLATFORM_OVERLOAD bool SaveKV3AsJSON( const KeyValues3* kv, CUtlString* error, CUtlString* output );
PLATFORM_OVERLOAD bool SaveKV3AsKV1Text( const KeyValues3* kv, CUtlString* error, CUtlBuffer* output, KV1TextEscapeBehavior_t esc_behavior );
PLATFORM_OVERLOAD bool SaveKV3AsKV1Text_Translated( const KeyValues3* kv, CUtlString* error, CUtlBuffer* output, KV1TextEscapeBehavior_t esc_behavior, const KV3ToKV1Translation_t* translation, int unk );
PLATFORM_OVERLOAD bool SaveKV3Text_NoHeader( const KeyValues3* kv, CUtlString* error, CBufferString* output, uint flags = KV3_SAVE_TEXT_NONE );
PLATFORM_OVERLOAD bool SaveKV3Text_NoHeader( const KeyValues3* kv, CUtlString* error, CUtlBuffer* output, uint flags = KV3_SAVE_TEXT_NONE );
PLATFORM_OVERLOAD bool SaveKV3Text_NoHeader( const KeyValues3* kv, CUtlString* error, CUtlString* output, uint flags = KV3_SAVE_TEXT_NONE );
PLATFORM_OVERLOAD bool SaveKV3Text_ToString( const KV3ID_t& format, const KeyValues3* kv, CUtlString* error, CBufferString* output, uint flags = KV3_SAVE_TEXT_NONE );
PLATFORM_OVERLOAD bool SaveKV3Text_ToString( const KV3ID_t& format, const KeyValues3* kv, CUtlString* error, CUtlString* output, uint flags = KV3_SAVE_TEXT_NONE );
PLATFORM_OVERLOAD bool SaveKV3ToFile( const KV3ID_t& encoding, const KV3ID_t& format, const KeyValues3* kv, CUtlString* error, const char* filename, const char* path, uint flags = KV3_SAVE_TEXT_NONE );

typedef int32 KV3MemberId_t;
#define KV3_INVALID_MEMBER ((KV3MemberId_t)-1)

// AMNOTE: These constants aren't actual constants, but rather calculated at compile time
// but the way they are calculated is unknown, previously it was using CUtlLeanVector min/max calculations
// but in here they seem to not match that behaviour.
enum
{
	ALLOC_KV3TABLE_MIN = 4,
	ALLOC_KV3TABLE_MAX = 0x6186154,

	ALLOC_KV3ARRAY_MIN = 4,
	ALLOC_KV3ARRAY_MAX = 0xFFFFF7F,

	ALLOC_CONTEXT_NODELIST_MIN = 32,
	ALLOC_CONTEXT_NODELIST_MAX = INT_MAX
};

enum
{
	KV3_ARRAY_MAX_FIXED_MEMBERS = 6,
	KV3_TABLE_MAX_FIXED_MEMBERS = 8,

	KV3_CONTEXT_SIZE = 4608,

	KV3_ARRAY_INIT_SIZE = 32,
	KV3_TABLE_INIT_SIZE = 64,

	KV3_CLUSTER_MAX_ELEMENTS = 253
};

enum KV3Type_t : uint8
{
	KV3_TYPE_INVALID = 0,
	KV3_TYPE_NULL,
	KV3_TYPE_BOOL,
	KV3_TYPE_INT,
	KV3_TYPE_UINT,
	KV3_TYPE_DOUBLE,
	KV3_TYPE_STRING,
	KV3_TYPE_BINARY_BLOB,
	KV3_TYPE_ARRAY,
	KV3_TYPE_TABLE,

	KV3_TYPE_COUNT,
};

enum KV3TypeOpt_t : uint8
{
	KV3_TYPEOPT_NONE = 0,
	
	KV3_TYPEOPT_STRING_SHORT,
	KV3_TYPEOPT_STRING_EXTERN,
	
	KV3_TYPEOPT_BINARY_BLOB_EXTERN,
	
	KV3_TYPEOPT_ARRAY_FLOAT32,
	KV3_TYPEOPT_ARRAY_FLOAT64,
	KV3_TYPEOPT_ARRAY_INT16,
	KV3_TYPEOPT_ARRAY_INT32,
	KV3_TYPEOPT_ARRAY_UINT8_SHORT,
	KV3_TYPEOPT_ARRAY_INT16_SHORT,
};

enum KV3TypeEx_t : uint8
{
	KV3_TYPEEX_INVALID = 0,
	KV3_TYPEEX_NULL,
	KV3_TYPEEX_BOOL,
	KV3_TYPEEX_INT,
	KV3_TYPEEX_UINT,
	KV3_TYPEEX_DOUBLE,

	KV3_TYPEEX_STRING			= KV3_TYPE_STRING,
	KV3_TYPEEX_STRING_SHORT		= (KV3_TYPEEX_STRING|(KV3_TYPEOPT_STRING_SHORT << 4)),
	KV3_TYPEEX_STRING_EXTERN	= (KV3_TYPEEX_STRING|(KV3_TYPEOPT_STRING_EXTERN << 4)),

	KV3_TYPEEX_BINARY_BLOB			= KV3_TYPE_BINARY_BLOB,
	KV3_TYPEEX_BINARY_BLOB_EXTERN	= (KV3_TYPEEX_BINARY_BLOB|(KV3_TYPEOPT_BINARY_BLOB_EXTERN << 4)),

	KV3_TYPEEX_ARRAY				= KV3_TYPE_ARRAY,
	KV3_TYPEEX_ARRAY_FLOAT32		= (KV3_TYPEEX_ARRAY|(KV3_TYPEOPT_ARRAY_FLOAT32 << 4)),
	KV3_TYPEEX_ARRAY_FLOAT64		= (KV3_TYPEEX_ARRAY|(KV3_TYPEOPT_ARRAY_FLOAT64 << 4)),
	KV3_TYPEEX_ARRAY_INT16			= (KV3_TYPEEX_ARRAY|(KV3_TYPEOPT_ARRAY_INT16 << 4)),
	KV3_TYPEEX_ARRAY_INT32			= (KV3_TYPEEX_ARRAY|(KV3_TYPEOPT_ARRAY_INT32 << 4)),
	KV3_TYPEEX_ARRAY_UINT8_SHORT	= (KV3_TYPEEX_ARRAY|(KV3_TYPEOPT_ARRAY_UINT8_SHORT << 4)),
	KV3_TYPEEX_ARRAY_INT16_SHORT	= (KV3_TYPEEX_ARRAY|(KV3_TYPEOPT_ARRAY_INT16_SHORT << 4)),

	KV3_TYPEEX_TABLE = KV3_TYPE_TABLE,
};

enum KV3SubType_t : uint8
{
	KV3_SUBTYPE_INVALID = 0,

	// string types
	KV3_SUBTYPE_RESOURCE,
	KV3_SUBTYPE_RESOURCE_NAME,
	KV3_SUBTYPE_PANORAMA,
	KV3_SUBTYPE_SOUNDEVENT,
	KV3_SUBTYPE_SUBCLASS, // table type
	KV3_SUBTYPE_ENTITY_NAME, // string type
	KV3_SUBTYPE_LOCALIZE,

	KV3_SUBTYPE_UNSPECIFIED,
	KV3_SUBTYPE_NULL,
	KV3_SUBTYPE_BINARY_BLOB,
	KV3_SUBTYPE_ARRAY,
	KV3_SUBTYPE_TABLE,
	KV3_SUBTYPE_BOOL8,
	KV3_SUBTYPE_CHAR8,
	KV3_SUBTYPE_UCHAR32,
	KV3_SUBTYPE_INT8,
	KV3_SUBTYPE_UINT8,
	KV3_SUBTYPE_INT16,
	KV3_SUBTYPE_UINT16,
	KV3_SUBTYPE_INT32,
	KV3_SUBTYPE_UINT32,
	KV3_SUBTYPE_INT64,
	KV3_SUBTYPE_UINT64,
	KV3_SUBTYPE_FLOAT32,
	KV3_SUBTYPE_FLOAT64,
	KV3_SUBTYPE_STRING,
	KV3_SUBTYPE_POINTER,
	KV3_SUBTYPE_COLOR32,

	// vector types
	KV3_SUBTYPE_VECTOR,
	KV3_SUBTYPE_VECTOR2D,
	KV3_SUBTYPE_VECTOR4D,
	KV3_SUBTYPE_ROTATION_VECTOR,
	KV3_SUBTYPE_QUATERNION,
	KV3_SUBTYPE_QANGLE,
	KV3_SUBTYPE_MATRIX3X4,
	KV3_SUBTYPE_TRANSFORM,

	KV3_SUBTYPE_STRING_TOKEN,
	KV3_SUBTYPE_EHANDLE,

	KV3_SUBTYPE_COUNT,
};

enum KV3ArrayAllocType_t
{
	KV3_ARRAY_ALLOC_EXTERN = 0,
	KV3_ARRAY_ALLOC_NORMAL = 1,
	KV3_ARRAY_ALLOC_EXTERN_FREE = 2,
};

enum KV3ToStringFlags_t
{
	KV3_TO_STRING_NONE = 0,
	KV3_TO_STRING_DONT_CLEAR_BUFF = (1 << 0),
	KV3_TO_STRING_DONT_APPEND_STRINGS = (1 << 1),
	KV3_TO_STRING_APPEND_ONLY_NUMERICS = (1 << 2),
	KV3_TO_STRING_RETURN_NON_NUMERICS = (1 << 3),
};

enum KV3MetaDataFlags_t
{
	KV3_METADATA_MULTILINE_STRING = (1 << 0),
	KV3_METADATA_SINGLE_QUOTED_STRING = (1 << 1),
};

namespace KV3Helpers
{
	template <typename T, typename... Ts>
	constexpr size_t PackAlignOf()
	{
		if constexpr (sizeof...(Ts) == 0)
			return alignof(T);
		else
			return (alignof(T) > PackAlignOf<Ts...>()) ? alignof(T) : PackAlignOf<Ts...>();
	}

	template <size_t ALIGN, typename... Ts>
	constexpr size_t PackSizeOf( int size )
	{
		return ((ALIGN_VALUE( size * sizeof( Ts ), ALIGN )) + ... + 0);
	}

	inline int CalcNewBufferSize( int old_size, int requested_size, int min_size, int max_size )
	{
		int new_size = MAX( old_size, min_size );

		while(new_size < requested_size)
		{
			if(new_size < max_size / 2)
				new_size *= 2;
			else
			{
				new_size = max_size;
				break;
			}
		}

		return new_size;
	}
}

struct KV3MetaData_t
{
	KV3MetaData_t() : m_nLine( 0 ), m_nColumn( 0 ), m_nFlags( 0 ) {}

	void Clear()
	{
		m_nLine = 0;
		m_nColumn = 0;
		m_nFlags = 0;
		m_sName = CUtlSymbolLarge();
		m_Comments.RemoveAll();
	}

	void Purge()
	{
		m_nLine = 0;
		m_nColumn = 0;
		m_nFlags = 0;
		m_sName = CUtlSymbolLarge();
		m_Comments.Purge();
	}

	typedef CUtlMap<int, CBufferStringGrowable<8>, int, CDefLess<int>> CommentsMap_t;

	int 			m_nLine;
	int 			m_nColumn;
	uint			m_nFlags;
	CUtlSymbolLarge m_sName;
	CommentsMap_t 	m_Comments;
};

struct KV3BinaryBlob_t
{
	size_t m_nSize;
	union
	{
		const byte*	m_pubData;
		byte		m_ubData[1];
	};
	bool m_bFreeMemory;
};

class CKV3MemberName
{
public:
	inline CKV3MemberName(const char* pszString): m_nHashCode(), m_pszString("")
	{	
		if (!pszString || !pszString[0])
			return;

		m_nHashCode = MakeStringToken( pszString );
		m_pszString = pszString;
	}

	inline CKV3MemberName(): m_nHashCode(), m_pszString("") {}
	inline CKV3MemberName(unsigned int nHashCode, const char* pszString = ""): m_nHashCode(nHashCode), m_pszString(pszString) {}

	inline unsigned int GetHashCode() const { return m_nHashCode.GetHashCode(); }
	inline const char* GetString() const { return m_pszString; }

private:
	CUtlStringToken m_nHashCode;
	const char* m_pszString;
};

template<size_t SIZE, typename T>
class CKeyValues3ClusterImpl;

using CKeyValues3Cluster = CKeyValues3ClusterImpl<KV3_CLUSTER_MAX_ELEMENTS, KeyValues3>;
using CKeyValues3TableCluster = CKeyValues3ClusterImpl<KV3_TABLE_INIT_SIZE, CKeyValues3Table>;
using CKeyValues3ArrayCluster = CKeyValues3ClusterImpl<KV3_ARRAY_INIT_SIZE, CKeyValues3Array>;

class KeyValues3
{
public:
	KeyValues3( KV3TypeEx_t type = KV3_TYPEEX_NULL, KV3SubType_t subtype = KV3_SUBTYPE_UNSPECIFIED );
	KeyValues3( int cluster_elem, KV3TypeEx_t type, KV3SubType_t subtype );
	~KeyValues3();

	CKeyValues3Context* GetContext() const;
	KV3MetaData_t* GetMetaData( CKeyValues3Context** ppCtx = nullptr ) const;

	KV3Type_t GetType() const		{ return ( KV3Type_t )( m_TypeEx & 0xF ); }
	KV3TypeEx_t GetTypeEx() const	{ return ( KV3TypeEx_t )m_TypeEx; }
	KV3SubType_t GetSubType() const	{ return ( KV3SubType_t )m_SubType; }

	const char* GetTypeAsString() const;
	const char* GetSubTypeAsString() const;

	const char* ToString( CBufferString& buff, uint flags = KV3_TO_STRING_NONE ) const;

	void SetToNull() { PrepareForType( KV3_TYPEEX_NULL, KV3_SUBTYPE_NULL ); }

	bool GetBool( bool defaultValue = false ) const			{ return GetValue<bool>( defaultValue ); }
	char8 GetChar( char8 defaultValue = 0 ) const			{ return GetValue<char8>( defaultValue ); }
	uchar32 GetUChar32( uchar32 defaultValue = 0 ) const	{ return GetValue<uint32>( defaultValue ); }
	int8 GetInt8( int8 defaultValue = 0 ) const				{ return GetValue<int8>( defaultValue ); }
	uint8 GetUInt8( uint8 defaultValue = 0 ) const			{ return GetValue<uint8>( defaultValue ); }
	int16 GetShort( int16 defaultValue = 0 ) const			{ return GetValue<int16>( defaultValue ); }
	uint16 GetUShort( uint16 defaultValue = 0 ) const		{ return GetValue<uint16>( defaultValue ); }
	int32 GetInt( int32 defaultValue = 0 ) const			{ return GetValue<int32>( defaultValue ); }
	uint32 GetUInt( uint32 defaultValue = 0 ) const			{ return GetValue<uint32>( defaultValue ); }
	int64 GetInt64( int64 defaultValue = 0 ) const			{ return GetValue<int64>( defaultValue ); }
	uint64 GetUInt64( uint64 defaultValue = 0 ) const		{ return GetValue<uint64>( defaultValue ); }
	float32 GetFloat( float32 defaultValue = 0.0f ) const	{ return GetValue<float32>( defaultValue ); }
	float64 GetDouble( float64 defaultValue = 0.0 ) const	{ return GetValue<float64>( defaultValue ); }

	void SetBool( bool value )		{ SetValue<bool>( value, KV3_TYPEEX_BOOL, KV3_SUBTYPE_BOOL8 ); }
	void SetChar( char8 value )		{ SetValue<char8>( value, KV3_TYPEEX_INT, KV3_SUBTYPE_CHAR8 ); }
	void SetUChar32( uchar32 value ){ SetValue<uint32>( value, KV3_TYPEEX_UINT, KV3_SUBTYPE_UCHAR32 ); }
	void SetInt8( int8 value )		{ SetValue<int8>( value, KV3_TYPEEX_INT, KV3_SUBTYPE_INT8 ); }
	void SetUInt8( uint8 value )	{ SetValue<uint8>( value, KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT8 ); }
	void SetShort( int16 value )	{ SetValue<int16>( value, KV3_TYPEEX_INT, KV3_SUBTYPE_INT16 ); }
	void SetUShort( uint16 value )	{ SetValue<uint16>( value, KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT16 ); }
	void SetInt( int32 value )		{ SetValue<int32>( value, KV3_TYPEEX_INT, KV3_SUBTYPE_INT32 ); }
	void SetUInt( uint32 value )	{ SetValue<uint32>( value, KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT32 ); }
	void SetInt64( int64 value )	{ SetValue<int64>( value, KV3_TYPEEX_INT, KV3_SUBTYPE_INT64 ); }
	void SetUInt64( uint64 value )	{ SetValue<uint64>( value, KV3_TYPEEX_UINT, KV3_SUBTYPE_UINT64 ); }
	void SetFloat( float32 value )	{ SetValue<float32>( value, KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT32 ); }
	void SetDouble( float64 value )	{ SetValue<float64>( value, KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT64 ); }

	void* GetPointer( void *defaultValue = ( void* )0 ) const { return ( GetSubType() == KV3_SUBTYPE_POINTER ) ? ( void* )m_Data.m_UInt : defaultValue; }
	void SetPointer( void* ptr ) { SetValue<uint64>( ( uint64 )ptr, KV3_TYPEEX_UINT, KV3_SUBTYPE_POINTER ); }
	
	CUtlStringToken GetStringToken( CUtlStringToken defaultValue = CUtlStringToken() ) const { return ( GetSubType() == KV3_SUBTYPE_STRING_TOKEN ) ? CUtlStringToken( ( uint32 )m_Data.m_UInt ) : defaultValue; }
	void SetStringToken( CUtlStringToken token ) { SetValue<uint32>( token.GetHashCode(), KV3_TYPEEX_UINT, KV3_SUBTYPE_STRING_TOKEN ); }

	CEntityHandle GetEHandle( CEntityHandle defaultValue = CEntityHandle() ) const { return ( GetSubType() == KV3_SUBTYPE_EHANDLE ) ? CEntityHandle( ( uint32 )m_Data.m_UInt ) : defaultValue; }
	void SetEHandle( CEntityHandle ehandle ) { SetValue<uint32>( ehandle.ToInt(), KV3_TYPEEX_UINT, KV3_SUBTYPE_EHANDLE ); }

	const char* GetString( const char *defaultValue = "" ) const;
	void SetString( const char* pString, KV3SubType_t subtype = KV3_SUBTYPE_STRING );
	void SetStringExternal( const char* pString, KV3SubType_t subtype = KV3_SUBTYPE_STRING );
	
	const byte* GetBinaryBlob() const;
	int GetBinaryBlobSize() const;
	void SetToBinaryBlob( const byte* blob, int size );
	void SetToBinaryBlobExternal( const byte* blob, int size, bool free_mem );

	Color GetColor( const Color &defaultValue = Color( 0, 0, 0, 255 ) ) const;
	void SetColor( const Color &color );

	Vector GetVector( const Vector &defaultValue = Vector( 0.0f, 0.0f, 0.0f ) ) const						{ return GetVecBasedObj<Vector>( 3, defaultValue ); }
	Vector2D GetVector2D( const Vector2D &defaultValue = Vector2D( 0.0f, 0.0f ) ) const						{ return GetVecBasedObj<Vector2D>( 2, defaultValue ); }
	Vector4D GetVector4D( const Vector4D &defaultValue = Vector4D( 0.0f, 0.0f, 0.0f, 0.0f ) ) const			{ return GetVecBasedObj<Vector4D>( 4, defaultValue ); }
	Quaternion GetQuaternion( const Quaternion &defaultValue = Quaternion( 0.0f, 0.0f, 0.0f, 0.0f ) ) const	{ return GetVecBasedObj<Quaternion>( 4, defaultValue ); }
	QAngle GetQAngle( const QAngle &defaultValue = QAngle( 0.0f, 0.0f, 0.0f ) ) const						{ return GetVecBasedObj<QAngle>( 3, defaultValue ); }
	matrix3x4_t GetMatrix3x4( const matrix3x4_t &defaultValue = matrix3x4_t( Vector( 0.0f, 0.0f, 0.0f ), Vector( 0.0f, 0.0f, 0.0f ), Vector( 0.0f, 0.0f, 0.0f ), Vector( 0.0f, 0.0f, 0.0f ) ) ) const { return GetVecBasedObj<matrix3x4_t>( 3*4, defaultValue ); }

	void SetVector( const Vector &vec )				{ SetVecBasedObj<Vector>( vec, 3, KV3_SUBTYPE_VECTOR ); }
	void SetVector2D( const Vector2D &vec2d )		{ SetVecBasedObj<Vector2D>( vec2d, 2, KV3_SUBTYPE_VECTOR2D ); }
	void SetVector4D( const Vector4D &vec4d )		{ SetVecBasedObj<Vector4D>( vec4d, 4, KV3_SUBTYPE_VECTOR4D ); }
	void SetQuaternion( const Quaternion &quat )	{ SetVecBasedObj<Quaternion>( quat, 4, KV3_SUBTYPE_QUATERNION ); }
	void SetQAngle( const QAngle &ang )				{ SetVecBasedObj<QAngle>( ang, 3, KV3_SUBTYPE_QANGLE ); }
	void SetMatrix3x4( const matrix3x4_t &matrix )	{ SetVecBasedObj<matrix3x4_t>( matrix, 3*4, KV3_SUBTYPE_MATRIX3X4 ); }

	int GetArrayElementCount() const;
	KeyValues3** GetArrayBase();
	KeyValues3* GetArrayElement( int elem );
	KeyValues3* InsertArrayElementBefore( int elem );
	KeyValues3* InsertArrayElementAfter( int elem ) { return InsertArrayElementBefore( elem + 1 ); }
	KeyValues3* AddArrayElementToTail();
	void SetArrayElementCount( int count, KV3TypeEx_t type = KV3_TYPEEX_NULL, KV3SubType_t subtype = KV3_SUBTYPE_UNSPECIFIED );
	void SetToEmptyArray() { SetArrayElementCount( 0 ); }
	void RemoveArrayElements( int elem, int num );
	void RemoveArrayElement( int elem ) { RemoveArrayElements( elem, 1 ); }

	int GetMemberCount() const;
	KeyValues3* GetMember( KV3MemberId_t id );
	const KeyValues3* GetMember( KV3MemberId_t id ) const { return const_cast<KeyValues3*>(this)->GetMember( id ); }
	const char* GetMemberName( KV3MemberId_t id ) const;
	CKV3MemberName GetMemberNameEx( KV3MemberId_t id ) const;
	unsigned int GetMemberHash( KV3MemberId_t id ) const;
	KeyValues3* FindMember( const CKV3MemberName &name, KeyValues3* defaultValue = NULL );
	KeyValues3* FindOrCreateMember( const CKV3MemberName &name, bool *pCreated = NULL );
	void SetToEmptyTable();
	bool RemoveMember( KV3MemberId_t id );
	bool RemoveMember( const KeyValues3* kv );
	bool RemoveMember( const CKV3MemberName &name );

	KeyValues3& operator=( const KeyValues3& src );
	
private:
	KeyValues3( const KeyValues3& other );

	union Data_t
	{
		Data_t() : m_nMemory(0)
		{
		}

		bool	m_Bool;
		int64	m_Int;
		uint64	m_UInt;
		float64	m_Double;

		const char* m_pString;
		char m_szStringShort[8];

		KV3BinaryBlob_t* m_pBinaryBlob;

		CKeyValues3Array* m_pArray;
		CKeyValues3Table* m_pTable;

		union Array_t
		{
			float32* m_f32;
			Vector *m_vec;
			Vector2D *m_vec2;
			Vector4D *m_vec4;
			Quaternion *m_quat;
			QAngle *m_ang;
			matrix3x4_t *m_mat;
			float64* m_f64;
			int16* m_i16;
			int32* m_i32;
			uint8 m_u8Short[8];
			int16 m_i16Short[4];
		} m_Array;

		uint64 m_nMemory;
		void* m_pMemory;
		char m_Memory[1];
	};

	void Alloc( int initial_size = 0, Data_t data = {}, int bytes_available = 0, bool should_free = false );

	CKeyValues3Array *AllocArray( int initial_size = 0 );
	CKeyValues3Table *AllocTable( int initial_size = 0 );

	void AllocArrayInPlace( int initial_size, Data_t data, int preallocated_size, bool should_free );
	void AllocTableInPlace( int initial_size, Data_t data, int preallocated_size, bool should_free );

	template <typename T>
	T *AllocateOnHeap( int initial_size = 0 );

	template <typename T>
	void FreeOnHeap( T *element );

	void FreeArray( CKeyValues3Array *element, bool clearing_context = false );
	void FreeTable( CKeyValues3Table *element, bool clearing_context = false );

	KeyValues3 *AllocMember( KV3TypeEx_t type = KV3_TYPEEX_NULL, KV3SubType_t subtype = KV3_SUBTYPE_UNSPECIFIED );
	void FreeMember( KeyValues3 *member );

	void Free( bool bClearingContext = false );
	void ResolveUnspecified();
	void PrepareForType( KV3TypeEx_t type, KV3SubType_t subtype );
	void OnClearContext();
	void CopyFrom( const KeyValues3* pSrc );

	int GetClusterElement() const { return m_nClusterElement; }
	void SetClusterElement( int element ) { m_nClusterElement = element; }
	CKeyValues3Cluster* GetCluster() const;

	template < typename T > T FromString( T defaultValue ) const;
	template < typename T > void SetDirect( T value );

	template < typename T > T GetValue( T defaultValue ) const;
	template < typename T > void SetValue( T value, KV3TypeEx_t type, KV3SubType_t subtype );

	template < typename T > T GetVecBasedObj( int size, const T &defaultValue ) const;
	template < typename T > void SetVecBasedObj( const T &obj, int size, KV3SubType_t subtype );

	template < typename T >
	void NormalizeArray( KV3TypeEx_t type, KV3SubType_t subtype, int size, const T* data, bool bFree );
	void NormalizeArray();

	template < typename T >
	void AllocArray( int size, const T* data, KV3ArrayAllocType_t alloc_type, KV3TypeEx_t type_short, KV3TypeEx_t type_ptr, KV3SubType_t subtype, KV3TypeEx_t type_elem, KV3SubType_t subtype_elem );

	bool ReadArrayInt32( int size, int32* data ) const;
	bool ReadArrayFloat32( int size, float32* data ) const;

	static constexpr size_t TotalSizeOf( int initial_size ) { return sizeof(KeyValues3); }
	static constexpr size_t TotalSizeOfData( int size ) { return sizeof(Data_t); }
	static constexpr size_t TotalSizeWithoutStaticData() { return sizeof(KeyValues3) - TotalSizeOfData( 0 ); }

private:
	uint64 m_bExternalStorage : 1;
	uint64 m_bFreeArrayMemory : 1;
	uint64 m_TypeEx : 8;
	uint64 m_SubType : 8;
	uint64 m_nFlags : 8;
	uint64 m_nClusterElement : 16;
	uint64 m_nNumArrayElements : 5;
	uint64 m_nReserved : 17;
	Data_t m_Data;

	friend CKeyValues3Cluster;
	friend CKeyValues3ArrayCluster;
	friend CKeyValues3TableCluster;
	friend class CKeyValues3Context;
	friend class CKeyValues3Table;
	friend class CKeyValues3Array;
};
COMPILE_TIME_ASSERT(sizeof(KeyValues3) == 16);

class CKeyValues3Array
{
public:
	typedef KeyValues3 *Element_t;

	static const size_t DATA_SIZE = KV3_ARRAY_MAX_FIXED_MEMBERS;
	static const size_t DATA_ALIGNMENT = KV3Helpers::PackAlignOf<Element_t>();

	CKeyValues3Array( int cluster_elem = -1, int alloc_size = DATA_SIZE );
	~CKeyValues3Array() { PurgeBuffers(); }

	int GetClusterElement() const { return m_nClusterElement; }
	void SetClusterElement( int element ) { m_nClusterElement = element; }

	CKeyValues3ArrayCluster* GetCluster() const;
	CKeyValues3Context* GetContext() const;

	Element_t *Base() { return IsBaseStatic() ? &m_StaticElements[0] : m_pDynamicElements; };
	Element_t const *Base() const { return const_cast<CKeyValues3Array *>(this)->Base(); }

	Element_t Element( int i );
	const Element_t Element( int i ) const { return const_cast<CKeyValues3Array*>(this)->Element( i ); }
	int Count() const { return m_nCount; }

	void EnsureElementCapacity( int count, bool force = false, bool dont_move = false );

	void SetCount( KeyValues3 *parent, int count, KV3TypeEx_t type = KV3_TYPEEX_NULL, KV3SubType_t subtype = KV3_SUBTYPE_UNSPECIFIED );
	Element_t* InsertMultipleBefore( KeyValues3 *parent, int from, int num );
	void CopyFrom( KeyValues3 *parent, const CKeyValues3Array* pSrc );
	void RemoveMultiple( KeyValues3 *parent, int from, int num );

	void OnClearContext() { PurgeContent( nullptr, true ); }
	void PurgeContent( KeyValues3 *parent = nullptr, bool clearing_context = false );
	void PurgeBuffers();

	static constexpr size_t TotalSizeOf( int initial_size ) { return ALIGN_VALUE( TotalSizeWithoutStaticData() + TotalSizeOfData( MAX( initial_size, 0 ) ), 8 ); }
	static constexpr size_t TotalSizeOfData( int size ) { return MAX( (KV3Helpers::PackSizeOf<DATA_ALIGNMENT, Element_t>( size )), sizeof( m_pDynamicElements ) ); }
	static constexpr size_t TotalSizeWithoutStaticData() { return sizeof( CKeyValues3Array ) - sizeof( m_StaticElements ); }

private:
	int GetAllocatedChunks() const { return m_nAllocatedChunks; }
	bool IsBaseStatic() { return !m_bIsDynamicallySized; }

	size_t GetAllocatedBytesSize() const { return TotalSizeOfData( GetAllocatedChunks() ); }

private:
	int m_nClusterElement;
	int m_nAllocatedChunks;

	int m_nCount;
	int8 m_nInitialSize;
	bool m_bIsDynamicallySized;

	bool m_unk001;
	bool m_unk002;

	union
	{
		Element_t m_StaticElements[DATA_SIZE];
		Element_t *m_pDynamicElements;
	};
};
COMPILE_TIME_ASSERT(sizeof(CKeyValues3Array) == 64);

class CKeyValues3Table
{
public:
	enum : uint8
	{
		TABLEFL_NONE = 0,
		TABLEFL_NAME_EXTERNAL = (1 << 0)
	};

	typedef uint32			Hash_t;
	typedef KeyValues3*		Member_t;
	typedef const char*		Name_t;
	typedef uint8			Flags_t;

	static const size_t DATA_SIZE = KV3_TABLE_MAX_FIXED_MEMBERS;
	static const size_t DATA_ALIGNMENT = KV3Helpers::PackAlignOf<Hash_t, Member_t, Name_t, Flags_t>();

	CKeyValues3Table( int cluster_elem = -1, int alloc_size = DATA_SIZE );
	~CKeyValues3Table() { PurgeBuffers(); }

	int GetClusterElement() const { return m_nClusterElement; }
	void SetClusterElement( int element ) { m_nClusterElement = element; }

	CKeyValues3TableCluster* GetCluster() const;
	CKeyValues3Context* GetContext() const;

	int GetMemberCount() const { return m_nCount; }
	Member_t GetMember( KV3MemberId_t id );
	const Member_t GetMember( KV3MemberId_t id ) const { return const_cast<CKeyValues3Table*>(this)->GetMember( id ); }
	const Name_t GetMemberName( KV3MemberId_t id ) const;
	const Hash_t GetMemberHash( KV3MemberId_t id ) const;

	void PurgeFastSearch();
	void EnableFastSearch();
	void EnsureMemberCapacity( int num, bool force = false, bool dont_move = false );

	KV3MemberId_t FindMember( const KeyValues3* kv ) const;
	KV3MemberId_t FindMember( const CKV3MemberName &name );
	KV3MemberId_t CreateMember( KeyValues3 *parent, const CKV3MemberName &name, bool name_external = false );

	void CopyFrom( KeyValues3 *parent, const CKeyValues3Table* src );
	void RemoveMember( KeyValues3 *parent, KV3MemberId_t id );
	void RemoveAll( KeyValues3 *parent, int new_size = 0 );

	void OnClearContext() { PurgeContent( nullptr, true ); }
	void PurgeContent( KeyValues3 *parent = nullptr, bool bClearingContext = false );
	void PurgeBuffers();

	static constexpr size_t TotalSizeOf( int initial_size ) { return ALIGN_VALUE( TotalSizeWithoutStaticData() + TotalSizeOfData( MAX( initial_size, 0 ) ), 8 ); }
	static constexpr size_t TotalSizeOfData( int size ) { return MAX( (KV3Helpers::PackSizeOf<DATA_ALIGNMENT, Hash_t, Member_t, Name_t, Flags_t>( size )), sizeof(m_pDynamicBuffer) ); }
	static constexpr size_t TotalSizeWithoutStaticData() { return sizeof(CKeyValues3Table) - sizeof(m_StaticBuffer); }

private:
	int GetAllocatedChunks() const { return m_nAllocatedChunks; }
	bool IsBaseStatic() { return !m_bIsDynamicallySized; }

	size_t GetAllocatedBytesSize() const { return TotalSizeOfData( GetAllocatedChunks() ); }

	constexpr size_t OffsetToHashesBase( int size ) const { return 0; }
	constexpr size_t OffsetToMembersBase( int size ) const { return KV3Helpers::PackSizeOf<DATA_ALIGNMENT, Hash_t>( size ); }
	constexpr size_t OffsetToNamesBase( int size ) const { return KV3Helpers::PackSizeOf<DATA_ALIGNMENT, Hash_t, Member_t>( size ); }
	constexpr size_t OffsetToFlagsBase( int size ) const { return KV3Helpers::PackSizeOf<DATA_ALIGNMENT, Hash_t, Member_t, Name_t>( size ); }

	// Gets the base address (can change when adding elements!)
	void *Base() { return IsBaseStatic() ? &m_StaticBuffer : m_pDynamicBuffer; };
	Hash_t *HashesBase() { return reinterpret_cast<Hash_t *>((uint8 *)Base() + OffsetToHashesBase( GetAllocatedChunks() )); }
	Member_t *MembersBase() { return reinterpret_cast<Member_t *>((uint8 *)Base() + OffsetToMembersBase( GetAllocatedChunks() )); }
	Name_t *NamesBase() { return reinterpret_cast<Name_t *>((uint8 *)Base() + OffsetToNamesBase( GetAllocatedChunks() )); }
	Flags_t *FlagsBase() { return reinterpret_cast<Flags_t *>((uint8 *)Base() + OffsetToFlagsBase( GetAllocatedChunks() )); }

	const void *Base() const { return const_cast<void *>(Base()); }
	const Hash_t *HashesBase() const { return const_cast<Hash_t *>(HashesBase()); }
	const Member_t *MembersBase() const { return const_cast<Member_t *>(MembersBase()); }
	const Name_t *NamesBase() const { return const_cast<Name_t *>(NamesBase()); }
	const Flags_t *FlagsBase() const { return const_cast<Flags_t *>(FlagsBase()); }

private:
	int m_nClusterElement;
	int m_nAllocatedChunks;

	struct kv3tablefastsearch_t {
		kv3tablefastsearch_t() : m_ignore( false ), m_ignores_counter( 0 ) {}
		~kv3tablefastsearch_t() { Clear(); }

		void Clear()
		{
			m_ignore = false;
			m_ignores_counter = 0;
			m_member_ids.RemoveAll();
		}

		struct EmptyHashFunctor { unsigned int operator()( uint32 n ) const { return n; } };
		typedef CUtlHashtable<unsigned int, KV3MemberId_t, EmptyHashFunctor> Hashtable_t;

		bool		m_ignore;
		int8		m_ignores_counter;
		Hashtable_t	m_member_ids;
	} *m_pFastSearch;

	int m_nCount;

	int8 m_nInitialSize;
	bool m_bIsDynamicallySized;

	bool m_unk001;
	bool m_unk002;

	union
	{
		struct
		{
			Hash_t m_Hashes[DATA_SIZE];
			Member_t m_Members[DATA_SIZE];
			Name_t m_Names[DATA_SIZE];
			Flags_t m_Flags[DATA_SIZE];
		} m_StaticBuffer;

		void* m_pDynamicBuffer;
	};
};
COMPILE_TIME_ASSERT(sizeof(CKeyValues3Table) == 192);

template <size_t SIZE, typename T>
class CKeyValues3ClusterImpl
{
public:
	typedef T NodeType;
	static const size_t SIZE = SIZE;

	union Node
	{
		Node() : m_pNextFree( nullptr ) {}
		~Node() {}

		NodeType m_Value;
		Node *m_pNextFree;
	};

	enum
	{
		HEAP_MARKER = (1 << 31),

		FLAGS_MASK = ~(HEAP_MARKER)
	};

	CKeyValues3ClusterImpl( CKeyValues3Context *context, bool allocated_on_heap = false, int initial_size = SIZE );
	~CKeyValues3ClusterImpl() { Purge(); }

	CKeyValues3Context *GetContext() const { return m_pContext; }

	bool IsFull() const { return NumCount() >= NumAllocated(); }
	bool IsAllocatedOnHeap() const { return (m_nAllocatedElements & HEAP_MARKER) != 0; }
	int NumAllocated() const { return m_nAllocatedElements & FLAGS_MASK; }
	int NumCount() const { return m_nElementCount; }

	template <typename... Args, typename = std::enable_if_t<std::is_constructible_v<NodeType, Args...>, int>>
	NodeType *Alloc( Args&&... args );

	void Free( int element, bool clearing_context = false );
	void Free( NodeType *node, bool clearing_context = false );

	void Purge();
	void Clear();

	Node *GetNextFree() const { return m_pFirstFreeNode; }
	void SetNextFree( Node *free ) { m_pFirstFreeNode = free; }

	CKeyValues3ClusterImpl *GetNext() const { return m_pNext; }
	void SetNext( CKeyValues3ClusterImpl *cluster ) { m_pNext = cluster; }

	CKeyValues3ClusterImpl *GetPrev() const { return m_pPrev; }
	void SetPrev( CKeyValues3ClusterImpl *cluster ) { m_pPrev = cluster; }

	Node *Head() { return &m_Values[0]; }
	Node *Tail() { return &m_Values[NumAllocated()]; }

	const Node *Head() const { return const_cast<CKeyValues3ClusterImpl *>(this)->Head(); }
	const Node *Tail() const { return const_cast<CKeyValues3ClusterImpl *>(this)->Tail(); }

	void EnableMetaData( bool bEnable );
	void ClearMetaData();
	void PurgeMetaData();
	void PurgeMetaData( int element );
	KV3MetaData_t *GetMetaData( int element ) const;

	int GetNodeIndex( NodeType *node ) const;

	static constexpr size_t TotalSizeOf( int initial_size ) { return ALIGN_VALUE( TotalSizeWithoutStaticData() + TotalSizeOfData( MAX( initial_size, 0 ) ), 8 ); }
	static constexpr size_t TotalSizeOfData( int size ) { return sizeof( Node ) * size; }
	static constexpr size_t TotalSizeWithoutStaticData() { return sizeof( CKeyValues3ClusterImpl ) - TotalSizeOfData( SIZE ); }

	friend CKeyValues3Cluster *KeyValues3::GetCluster() const;
	friend CKeyValues3ArrayCluster *CKeyValues3Array::GetCluster() const;
	friend CKeyValues3TableCluster *CKeyValues3Table::GetCluster() const;

private:
	void InitNodes();
	void PurgeNodes( bool clearing_context = false );

private:
	struct kv3metadata_t
	{
		int m_AllocatedElements;
		KV3MetaData_t m_elements[SIZE];
	};

	CKeyValues3Context *m_pContext;
	Node *m_pFirstFreeNode;

	int m_nAllocatedElements;
	int m_nElementCount;

	CKeyValues3ClusterImpl *m_pPrev;
	CKeyValues3ClusterImpl *m_pNext;

	kv3metadata_t *m_pMetaData;

	Node m_Values[SIZE];
};

class CKeyValues3ContextBase
{
public:
	CKeyValues3ContextBase( CKeyValues3Context* context );
	~CKeyValues3ContextBase() { Purge(); }

	void Clear();
	void Purge();

protected:
	template <typename CLUSTER>
	struct ClusterNodeChain
	{
		ClusterNodeChain() : m_pTail( nullptr ), m_pHead( nullptr )
		{}

		void Reset()
		{
			m_pTail = nullptr;
			m_pHead = nullptr;
		}

		void AddToChain( CLUSTER *cluster );
		void RemoveFromChain( CLUSTER *cluster );

		CLUSTER *m_pTail;
		CLUSTER *m_pHead;
	};

	template <typename NODE>
	class NodeList
	{
	public:
		struct ListEntry
		{
			ListEntry *m_pNext;
			NODE m_Value;
		};

		NodeList() : m_nUsedBytes( 0 ), m_nAllocatedBytes( 0 ), m_pData( nullptr )
		{}

		~NodeList() { Free(); }

		NODE *Alloc( int initial_size );
		void Free() { Purge(); }
		void Purge();
		void Clear();

		int UsedBytes() const { return m_nUsedBytes; }
		int AllocatedBytes() const { return m_nAllocatedBytes; }
		int FreeBytes() const { return m_nAllocatedBytes - m_nUsedBytes; }

		bool IsFull() const { return m_nUsedBytes >= m_nAllocatedBytes; }

		ListEntry *Head() { return &m_pData[0]; }
		ListEntry *Tail() { return reinterpret_cast<ListEntry *>((uint8 *)Head() + m_nUsedBytes); }

		bool IsWithinRange( NODE *element ) { return AllocatedBytes() > 0 && element >= (void *)Head() && element < (void *)Tail(); }

	private:
		void EnsureByteSize( int bytes_needed );

	private:
		int m_nUsedBytes;
		int m_nAllocatedBytes;
		ListEntry *m_pData;
	};

	CKeyValues3Context* m_pContext;
	CUtlBuffer m_BinaryData;

	CKeyValues3Cluster m_KV3BaseCluster;

	ClusterNodeChain<CKeyValues3Cluster> m_KV3PartialClusters;
	ClusterNodeChain<CKeyValues3Cluster> m_KV3FullClusters;

	ClusterNodeChain<CKeyValues3ArrayCluster> m_PartialArrayClusters;
	ClusterNodeChain<CKeyValues3ArrayCluster> m_FullArrayClusters;
	NodeList<CKeyValues3Array> m_RawArrayEntries;

	ClusterNodeChain<CKeyValues3TableCluster> m_PartialTableClusters;
	ClusterNodeChain<CKeyValues3TableCluster> m_FullTableClusters;
	NodeList<CKeyValues3Table> m_RawTableEntries;

	CUtlSymbolTableLarge m_Symbols;

	bool m_bMetaDataEnabled: 1;
	bool m_bFormatConverted: 1;
	bool m_bRootAvailabe: 1;

	IParsingErrorListener* m_pParsingErrorListener;

	friend class KeyValues3;
};

class CKeyValues3Context : public CKeyValues3ContextBase
{
	typedef CKeyValues3ContextBase BaseClass;

public:
	CKeyValues3Context( bool bNoRoot = false );
	~CKeyValues3Context() { Purge(); }

	KeyValues3* AllocKV( KV3TypeEx_t type = KV3_TYPEEX_NULL, KV3SubType_t subtype = KV3_SUBTYPE_UNSPECIFIED );
	// WARNING: kv must belong to this context!!!
	void FreeKV( KeyValues3* kv );
	
	// gets the pre-allocated kv if we indicated its existence when creating the context
	KeyValues3* Root();
	const KeyValues3* Root() const { return const_cast<CKeyValues3Context*>(this)->Root(); }

	bool IsMetaDataEnabled() const { return m_bMetaDataEnabled; }
	// returns true if the desired format was converted to another after loading via LoadKV3*
	bool IsFormatConverted() const { return m_bFormatConverted; }
	bool IsRootAvailabe() const { return m_bRootAvailabe; }

	// filled in after loading via LoadKV3* in binary encoding
	CUtlBuffer& GetBinaryData() { return m_BinaryData; }

	IParsingErrorListener* GetParsingErrorListener() const { return m_pParsingErrorListener; }
	void SetParsingErrorListener( IParsingErrorListener* listener ) { m_pParsingErrorListener = listener; }

	const char* AllocString( const char* pString );

	void EnableMetaData( bool bEnable );
	void CopyMetaData( KV3MetaData_t* pDest, const KV3MetaData_t* pSrc );

	void Clear();
	void Purge();

	template <typename CLUSTER>
	void ClearClusterNodeChain( ClusterNodeChain<CLUSTER> &cluster_node );
	template <typename CLUSTER>
	void PurgeClusterNodeChain( ClusterNodeChain<CLUSTER> &cluster_node );

	bool IsArrayRawAllocated( CKeyValues3Array *element ) { return m_RawArrayEntries.IsWithinRange( element ); }
	bool IsTableRawAllocated( CKeyValues3Table *element ) { return m_RawTableEntries.IsWithinRange( element ); }

private:
	template <typename CLUSTER>
	void MoveToPartial( ClusterNodeChain<CLUSTER> &full_cluster, ClusterNodeChain<CLUSTER> &partial_cluster );

	template <typename CLUSTER, typename... Args, typename = typename std::enable_if_t<std::is_constructible_v<CLUSTER::NodeType, Args...>, int>>
	auto Alloc( ClusterNodeChain<CLUSTER> &partial_clusters, ClusterNodeChain<CLUSTER> &full_clusters, int initial_size = CLUSTER::SIZE, Args&&... args );

	template <typename CLUSTER, typename NODE, typename... Args, typename = typename std::enable_if_t<std::is_constructible_v<CLUSTER::NodeType, Args...>, int>>
	NODE *RawAlloc( NodeList<NODE> &raw_array, ClusterNodeChain<CLUSTER> &partial_clusters, ClusterNodeChain<CLUSTER> &full_clusters, int initial_size = CLUSTER::SIZE, Args&&... args );

	CKeyValues3Array *AllocArray( int initial_size = 0 ) { return RawAlloc( m_RawArrayEntries, m_PartialArrayClusters, m_FullArrayClusters, initial_size ); }
	CKeyValues3Table *AllocTable( int initial_size = 0 ) { return RawAlloc( m_RawTableEntries, m_PartialTableClusters, m_FullTableClusters, initial_size ); }

	template<typename CLUSTER, typename NODE>
	void Free( NODE *element, ClusterNodeChain<CLUSTER> &partial_clusters, ClusterNodeChain<CLUSTER> &full_clusters );

	inline void FreeArray( CKeyValues3Array *element ) { Free( element, m_PartialArrayClusters, m_FullArrayClusters ); }
	inline void FreeTable( CKeyValues3Table *element ) { Free( element, m_PartialTableClusters, m_FullTableClusters ); }

private:
	uint8 pad[ KV3_CONTEXT_SIZE - ( sizeof( BaseClass ) % KV3_CONTEXT_SIZE ) ];

	friend class KeyValues3;
};
COMPILE_TIME_ASSERT(sizeof(CKeyValues3Context) == KV3_CONTEXT_SIZE);

template < typename T > inline T KeyValues3::FromString( T defaultValue ) const { Assert( 0 ); return defaultValue; }
template <> inline bool KeyValues3::FromString( bool defaultValue ) const		{ return V_StringToBool( GetString(), defaultValue ); }
template <> inline char8 KeyValues3::FromString( char8 defaultValue ) const		{ return V_StringToInt8( GetString(), defaultValue ); }
template <> inline int8 KeyValues3::FromString( int8 defaultValue ) const		{ return V_StringToInt8( GetString(), defaultValue ); }
template <> inline uint8 KeyValues3::FromString( uint8 defaultValue ) const		{ return V_StringToUint8( GetString(), defaultValue ); }
template <> inline int16 KeyValues3::FromString( int16 defaultValue ) const		{ return V_StringToInt16( GetString(), defaultValue ); }
template <> inline uint16 KeyValues3::FromString( uint16 defaultValue ) const	{ return V_StringToUint16( GetString(), defaultValue ); }
template <> inline int32 KeyValues3::FromString( int32 defaultValue ) const		{ return V_StringToInt32( GetString(), defaultValue ); }
template <> inline uint32 KeyValues3::FromString( uint32 defaultValue ) const	{ return V_StringToUint32( GetString(), defaultValue ); }
template <> inline int64 KeyValues3::FromString( int64 defaultValue ) const		{ return V_StringToInt64( GetString(), defaultValue ); }
template <> inline uint64 KeyValues3::FromString( uint64 defaultValue ) const	{ return V_StringToUint64( GetString(), defaultValue ); }
template <> inline float32 KeyValues3::FromString( float32 defaultValue ) const	{ return V_StringToFloat32( GetString(), defaultValue ); }
template <> inline float64 KeyValues3::FromString( float64 defaultValue ) const	{ return V_StringToFloat64( GetString(), defaultValue ); }

template < typename T > inline void KeyValues3::SetDirect( T value ) { Assert( 0 ); }
template <> inline void KeyValues3::SetDirect( bool value )		{ m_Data.m_Bool = value; }
template <> inline void KeyValues3::SetDirect( char8 value )	{ m_Data.m_Int = ( int64 )value; }
template <> inline void KeyValues3::SetDirect( int8 value )		{ m_Data.m_Int = ( int64 )value; }
template <> inline void KeyValues3::SetDirect( uint8 value )	{ m_Data.m_UInt = ( uint64 )value; }
template <> inline void KeyValues3::SetDirect( int16 value )	{ m_Data.m_Int = ( int64 )value; }
template <> inline void KeyValues3::SetDirect( uint16 value )	{ m_Data.m_UInt = ( uint64 )value; }
template <> inline void KeyValues3::SetDirect( int32 value )	{ m_Data.m_Int = ( int64 )value; }
template <> inline void KeyValues3::SetDirect( uint32 value )	{ m_Data.m_UInt = ( uint64 )value; }
template <> inline void KeyValues3::SetDirect( int64 value )	{ m_Data.m_Int = value; }
template <> inline void KeyValues3::SetDirect( uint64 value )	{ m_Data.m_UInt = value; }
template <> inline void KeyValues3::SetDirect( float32 value )	{ m_Data.m_Double = ( float64 )value; }
template <> inline void KeyValues3::SetDirect( float64 value )	{ m_Data.m_Double = value; }

template < typename T >
T KeyValues3::GetVecBasedObj( int size, const T &defaultValue ) const
{
	T obj;
	if ( !ReadArrayFloat32( size, obj.Base() ) )
		obj = defaultValue;
	return obj;
}

template < typename T >
void KeyValues3::SetVecBasedObj( const T &obj, int size, KV3SubType_t subtype )
{
	AllocArray<float32>( size, obj.Base(), KV3_ARRAY_ALLOC_NORMAL, KV3_TYPEEX_INVALID, KV3_TYPEEX_ARRAY_FLOAT32, subtype, KV3_TYPEEX_DOUBLE, KV3_SUBTYPE_FLOAT32 );
}

template < typename T >
T KeyValues3::GetValue( T defaultValue ) const
{
	switch ( GetType() )
	{
		case KV3_TYPE_BOOL:
			return ( T )m_Data.m_Bool;
		case KV3_TYPE_INT:
			return ( T )m_Data.m_Int;
		case KV3_TYPE_UINT:
			return ( GetSubType() != KV3_SUBTYPE_POINTER ) ? ( T )m_Data.m_UInt : defaultValue;
		case KV3_TYPE_DOUBLE:
			return ( T )m_Data.m_Double;
		case KV3_TYPE_STRING:
			return FromString<T>( defaultValue );
		default:
			return defaultValue;
	}
}

template < typename T >
void KeyValues3::SetValue( T value, KV3TypeEx_t type, KV3SubType_t subtype )
{
	PrepareForType( type, subtype );
	SetDirect<T>( value );
}

template < typename T >
void KeyValues3::NormalizeArray( KV3TypeEx_t type, KV3SubType_t subtype, int size, const T* data, bool bFree )
{
	m_TypeEx = KV3_TYPEEX_ARRAY;
	Alloc( size );

	m_Data.m_pArray->SetCount( this, size, type, subtype );

	CKeyValues3Array::Element_t* arr = m_Data.m_pArray->Base();
	for ( int i = 0; i < m_Data.m_pArray->Count(); ++i )
		arr[ i ]->SetDirect( data[ i ] );

	if ( bFree )
		free( (void*)data );
}

template<typename T>
inline T *KeyValues3::AllocateOnHeap( int initial_size )
{
	if(initial_size <= 0)
		initial_size = T::DATA_SIZE;

	auto element = (T *)g_pMemAlloc->RegionAlloc( MEMALLOC_REGION_ALLOC_4, T::TotalSizeOf( initial_size ) );
	Construct( element, -1, initial_size );

	return element;
}

template<typename T>
inline void KeyValues3::FreeOnHeap( T *element )
{
	Destruct( element );

	g_pMemAlloc->RegionFree( MEMALLOC_REGION_FREE_4, element );
}

template < typename T >
void KeyValues3::AllocArray( int size, const T* data, KV3ArrayAllocType_t alloc_type, KV3TypeEx_t type_short, KV3TypeEx_t type_ptr, KV3SubType_t subtype, KV3TypeEx_t type_elem, KV3SubType_t subtype_elem )
{
	int nMaxSizeShort = sizeof( uint64 ) / sizeof( T );

	if ( type_short != KV3_TYPEEX_INVALID && size <= nMaxSizeShort )
	{
		if ( alloc_type == KV3_ARRAY_ALLOC_EXTERN && type_ptr != KV3_TYPEEX_INVALID )
		{
			PrepareForType( type_ptr, subtype );

			m_bFreeArrayMemory = false;
			m_nNumArrayElements = size;
			m_Data.m_pMemory = (void*)data;
		}
		else
		{
			PrepareForType( type_short, subtype );

			m_bFreeArrayMemory = false;
			m_nNumArrayElements = size;
			m_Data.m_pMemory = NULL;
			memcpy( &m_Data.m_pMemory, data, size * sizeof( T ) );

			if ( alloc_type == KV3_ARRAY_ALLOC_EXTERN_FREE )
				free( (void*)data );
		}
	}
	else if ( type_ptr != KV3_TYPEEX_INVALID && size <= 31 )
	{
		PrepareForType( type_ptr, subtype );

		m_nNumArrayElements = size;

		if ( alloc_type == KV3_ARRAY_ALLOC_EXTERN )
		{
			m_bFreeArrayMemory = false;
			m_Data.m_pMemory = (void*)data;
		}
		else if ( alloc_type == KV3_ARRAY_ALLOC_EXTERN_FREE )
		{
			m_bFreeArrayMemory = true;
			m_Data.m_pMemory = (void*)data;
		}
		else
		{
			m_bFreeArrayMemory = true;	
			m_Data.m_pMemory = malloc( size * sizeof( T ) );
			memcpy( m_Data.m_pMemory, data, size * sizeof( T ) );
		}
	}
	else
	{
		PrepareForType( KV3_TYPEEX_ARRAY, subtype );

		m_Data.m_pArray->SetCount( this, size, type_elem, subtype_elem );

		CKeyValues3Array::Element_t* arr = m_Data.m_pArray->Base();
		for ( int i = 0; i < m_Data.m_pArray->Count(); ++i )
			arr[ i ]->SetValue<T>( data[ i ], type_elem, subtype_elem );

		if ( alloc_type == KV3_ARRAY_ALLOC_EXTERN_FREE )
			free( (void*)data );
	}
}

template<size_t SIZE, typename T>
inline CKeyValues3ClusterImpl<SIZE, T>::CKeyValues3ClusterImpl( CKeyValues3Context *context, bool allocated_on_heap, int initial_size ) :
	m_pContext( context ),
	m_pFirstFreeNode( nullptr ),
	m_nAllocatedElements( initial_size | (allocated_on_heap ? HEAP_MARKER : 0) ),
	m_nElementCount( 0 ),
	m_pPrev( nullptr ),
	m_pNext( nullptr ),
	m_pMetaData( nullptr )
{
	InitNodes();
}

template<size_t SIZE, typename T>
template<typename... Args, typename>
inline T *CKeyValues3ClusterImpl<SIZE, T>::Alloc( Args&&... args )
{
	Assert( !IsFull() );

	Node *node = GetNextFree();
	Assert( node != nullptr );

	SetNextFree( node->m_pNextFree );

	Construct( &node->m_Value, std::forward<Args>( args )... );
	node->m_Value.SetClusterElement( GetNodeIndex( &node->m_Value ) );

	m_nElementCount++;

	return &node->m_Value;
}

template<size_t SIZE, typename T>
inline void CKeyValues3ClusterImpl<SIZE, T>::Free( NodeType* node, bool clearing_context )
{
	Assert( node >= Head() && node < Tail() );
	Free( GetNodeIndex( node ), clearing_context );
}

template<size_t SIZE, typename T>
inline void CKeyValues3ClusterImpl<SIZE, T>::Free( int element, bool clearing_context )
{
	Assert( element >= 0 && element < NumAllocated() );

	Node *node = &m_Values[element];

	if(clearing_context)
		node->m_Value.OnClearContext();
	else
		Destruct( &node->m_Value );

	m_nElementCount--;

	node->m_pNextFree = GetNextFree();
	SetNextFree( node );
}

template<size_t SIZE, typename T>
inline void CKeyValues3ClusterImpl<SIZE, T>::InitNodes()
{
	Node *iter = Tail() - 1;
	Node *prev = nullptr;

	for(int i = 0; i < NumAllocated(); i++, iter--)
	{
		iter->m_pNextFree = prev;
		prev = iter;
	}

	m_nElementCount = 0;
	SetNextFree( prev );
}

template<size_t SIZE, typename T>
inline void CKeyValues3ClusterImpl<SIZE, T>::PurgeNodes( bool clearing_context )
{
	CVarBitVec free_nodes( NumAllocated() * BITS_PER_INT );

	for(auto iter = GetNextFree(); iter; iter = iter->m_pNextFree)
	{
		free_nodes.Set( GetNodeIndex( &iter->m_Value ) );
	}

	if(!free_nodes.IsAllSet())
	{
		for(int i = 0; i < NumAllocated(); i++)
		{
			if(!free_nodes.IsBitSet( i ))
			{
				Free( i, clearing_context );
			}
		}

		InitNodes();
	}
}

template<size_t SIZE, typename T>
inline int CKeyValues3ClusterImpl<SIZE, T>::GetNodeIndex( NodeType *element ) const
{
	Node *node = reinterpret_cast<Node *>(element);

	auto head = Head();
	if(node < head || node >= Tail())
		return -1;

	return node - head;
}

template<size_t SIZE, typename T>
inline void CKeyValues3ClusterImpl<SIZE, T>::Purge()
{
	PurgeNodes( false );
	PurgeMetaData();
}

template<size_t SIZE, typename T>
inline void CKeyValues3ClusterImpl<SIZE, T>::Clear()
{
	PurgeNodes( true );
	ClearMetaData();
}

template<size_t SIZE, typename T>
void CKeyValues3ClusterImpl<SIZE, T>::EnableMetaData( bool bEnable )
{
	if(bEnable)
	{
		if(!m_pMetaData)
		{
			m_pMetaData = (kv3metadata_t *)g_pMemAlloc->RegionAlloc( MEMALLOC_REGION_ALLOC_4, (NumAllocated() * sizeof(KV3MetaData_t)) + 8 );
			m_pMetaData->m_AllocatedElements = NumAllocated();
		}
	}
	else
	{
		PurgeMetaData();
	}
}

template<size_t SIZE, typename T>
void CKeyValues3ClusterImpl<SIZE, T>::ClearMetaData()
{
	if(m_pMetaData)
	{
		for(int i = 0; i < m_pMetaData->m_AllocatedElements; i++)
		{
			m_pMetaData->m_elements[i].Clear();
		}
	}
}

template<size_t SIZE, typename T>
void CKeyValues3ClusterImpl<SIZE, T>::PurgeMetaData()
{
	if(m_pMetaData)
	{
		for(int i = 0; i < m_pMetaData->m_AllocatedElements; i++)
		{
			m_pMetaData->m_elements[i].Purge();
		}

		g_pMemAlloc->RegionFree( MEMALLOC_REGION_FREE_4, m_pMetaData );
	}

	m_pMetaData = nullptr;
}

template<size_t SIZE, typename T>
void CKeyValues3ClusterImpl<SIZE, T>::PurgeMetaData( int element )
{
	if(!m_pMetaData)
		return;

	Assert( element >= 0 && element < m_pMetaData->m_AllocatedElements );
	GetMetaData( element )->Clear();
}

template<size_t SIZE, typename T>
KV3MetaData_t *CKeyValues3ClusterImpl<SIZE, T>::GetMetaData( int element ) const
{
	if(!m_pMetaData)
		return nullptr;

	Assert( element >= 0 && element < m_pMetaData->m_AllocatedElements );
	return &m_pMetaData->m_elements[element];
}

template<typename CLUSTER>
inline void CKeyValues3ContextBase::ClusterNodeChain<CLUSTER>::AddToChain( CLUSTER *cluster )
{
	if(m_pTail)
		m_pTail->SetNext( cluster );
	else
		m_pHead = cluster;

	cluster->SetNext( nullptr );
	cluster->SetPrev( m_pTail );

	m_pTail = cluster;
}

template<typename CLUSTER>
inline void CKeyValues3ContextBase::ClusterNodeChain<CLUSTER>::RemoveFromChain( CLUSTER *cluster )
{
	auto prev = cluster->GetPrev();
	auto next = cluster->GetNext();

	if(prev)
		prev->SetNext( next );
	else
		m_pHead = next;

	if(next)
		next->SetPrev( prev );
	else
		m_pTail = prev;

	cluster->SetPrev( nullptr );
	cluster->SetNext( nullptr );
}

template<typename NODE>
inline void CKeyValues3ContextBase::NodeList<NODE>::EnsureByteSize( int bytes_needed )
{
	if(bytes_needed < m_nAllocatedBytes)
		return;

	int new_alloc_size = KV3Helpers::CalcNewBufferSize( m_nAllocatedBytes, bytes_needed, ALLOC_CONTEXT_NODELIST_MIN, ALLOC_CONTEXT_NODELIST_MAX );

	m_pData = (ListEntry *)realloc( m_pData, new_alloc_size );
	m_nAllocatedBytes = new_alloc_size;
}

template<typename NODE>
inline NODE *CKeyValues3ContextBase::NodeList<NODE>::Alloc( int initial_size )
{
	int byte_size_needed = m_nUsedBytes + NODE::TotalSizeOf( initial_size ) + 8;
	EnsureByteSize( byte_size_needed );
	
	auto entry = Tail();
	m_nUsedBytes = byte_size_needed;

	Construct( &entry->m_Value, -1, initial_size );
	entry->m_pNext = Tail();

	return &entry->m_Value;
}

template<typename NODE>
inline void CKeyValues3ContextBase::NodeList<NODE>::Clear()
{
	if(m_nAllocatedBytes > 0)
	{
		for(auto iter = Head(); iter; iter = iter->m_pNext)
		{
			Destruct( iter );
		}
	}

	m_nUsedBytes = 0;
}

template<typename NODE>
inline void CKeyValues3ContextBase::NodeList<NODE>::Purge()
{
	Clear();

	free( m_pData );
	m_pData = nullptr;
}

template<typename CLUSTER>
inline void CKeyValues3Context::PurgeClusterNodeChain( ClusterNodeChain<CLUSTER> &cluster_node )
{
	for(auto node = cluster_node.m_pTail; node; node = node->GetPrev())
	{
		if(node->IsAllocatedOnHeap())
		{
			node->Purge();
			g_pMemAlloc->RegionFree( MEMALLOC_REGION_FREE_4, node );
		}
		else
		{
			node->Clear();
		}
	}

	cluster_node.Reset();
}

template<typename CLUSTER>
inline void CKeyValues3Context::ClearClusterNodeChain( ClusterNodeChain<CLUSTER> &cluster_node )
{
	for(auto node = cluster_node.m_pTail; node; node = node->GetPrev())
	{
		node->Clear();
	}
}

template<typename CLUSTER>
inline void CKeyValues3Context::MoveToPartial( ClusterNodeChain<CLUSTER> &full_cluster, ClusterNodeChain<CLUSTER> &partial_cluster )
{
	for(auto node = full_cluster.m_pTail; node; node = node->GetPrev())
	{
		partial_cluster.AddToChain( node );
	}

	full_cluster.Reset();
}

template <typename CLUSTER, typename... Args, typename>
auto CKeyValues3Context::Alloc( ClusterNodeChain<CLUSTER> &partial_clusters,
								ClusterNodeChain<CLUSTER> &full_clusters,
								int initial_size, Args&&... args )
{
	auto tail = partial_clusters.m_pTail;
	CLUSTER::NodeType *elem = nullptr;

	if(tail)
	{
		elem = tail->Alloc( std::forward<Args>( args )... );

		if(tail->IsFull())
		{
			partial_clusters.RemoveFromChain( tail );
			full_clusters.AddToChain( tail );
		}
	}
	else
	{
		auto cluster = (CLUSTER *)g_pMemAlloc->RegionAlloc( MEMALLOC_REGION_ALLOC_4, CLUSTER::TotalSizeOf( initial_size ) );

		Construct( cluster, this, true, initial_size );
		partial_clusters.AddToChain( cluster );

		elem = cluster->Alloc( std::forward<Args>( args )... );
	}

	return elem;
}

template<typename CLUSTER, typename NODE, typename ...Args, typename>
inline NODE *CKeyValues3Context::RawAlloc( NodeList<NODE> &raw_array, ClusterNodeChain<CLUSTER> &partial_clusters, ClusterNodeChain<CLUSTER> &full_clusters, int initial_size, Args && ...args )
{
	int needed_byte_size = MAX( NODE::TotalSizeOf( initial_size ), 32 );

	if(raw_array.IsFull() || needed_byte_size > raw_array.FreeBytes())
	{
		if(initial_size <= NODE::DATA_SIZE)
			return Alloc( partial_clusters, full_clusters );
		else
			return nullptr;
	}

	return raw_array.Alloc( initial_size );
}

template<typename CLUSTER, typename NODE>
void CKeyValues3Context::Free( NODE *element, ClusterNodeChain<CLUSTER> &partial_clusters, ClusterNodeChain<CLUSTER> &full_clusters )
{
	auto cluster = element->GetCluster();

	Assert( cluster != nullptr && cluster->GetContext() == m_pContext );
	
	cluster->Free( element );

	int num_allocated = cluster->NumAllocated();

	if(cluster->NumCount() > 0)
	{
		if(cluster->NumCount() == cluster->NumAllocated() - 1)
		{
			full_clusters.RemoveFromChain( cluster );
			partial_clusters.AddToChain( cluster );
		}
	}
	else if(cluster->IsAllocatedOnHeap())
	{
		partial_clusters.RemoveFromChain( cluster );

		Destruct( cluster );
		g_pMemAlloc->RegionFree( MEMALLOC_REGION_FREE_4, cluster );
	}
}

#include "tier0/memdbgoff.h"

#endif // KEYVALUES3_H
