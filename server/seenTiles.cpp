

#include "kissdb.h"
#include "lineardb3.h"

#include "dbCommon.h"
#include "minorGems/system/Time.h"

#include <malloc.h>
#include <math.h>

#include "minorGems/util/random/CustomRandomSource.h"

#ifdef _WIN32
#define fseeko fseeko64
#define ftello ftello64
#endif

#define DB_name "LinearDB3"
#define DB LINEARDB3
#define DB_open LINEARDB3_open
#define DB_close LINEARDB3_close
#define DB_get LINEARDB3_get
#define DB_put LINEARDB3_put
// linear db has no put_new
#define DB_put_new LINEARDB3_put
#define DB_Iterator  LINEARDB3_Iterator
#define DB_Iterator_init  LINEARDB3_Iterator_init
#define DB_Iterator_next  LINEARDB3_Iterator_next
#define DB_maxStack db.maxOverflowDepth

void toTileCoord( int x, int y, int tileWidth, int* tileX, int* tileY ) {
    int modX = x % tileWidth;
    if( modX < 0 ) {
        *tileX = (x - tileWidth - modX) / tileWidth;
        }
    else if( modX > 0 ) {
        *tileX = (x - modX) / tileWidth;
        }

    int modY = y % tileWidth;
    if( modY < 0 ) {
        *tileY = -(y - modY) / tileWidth;
        }
    else if( modY > 0 ) {
        *tileY = -(y + tileWidth - modY) / tileWidth;
        }
    }




void testCoordTranslation2() {
    int tileWidth = 2;
    printf( "tileWidth %d\n", tileWidth );

    int length;
    printf(" xxxxxxxxxxxxxxxxxxxxxxxxx\n");
    int inputx[] =  { 5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6};
    int outputx[] = { 2,  2,  1,  1,  0,  0, -1, -1, -2, -2, -3, -3};
    length = sizeof(inputx) / sizeof(int);
    for( int i = 0; i < length; i++ ) {
        int x = inputx[i];
        int tileX;
        int tileY;
        toTileCoord( x, 0, tileWidth, &tileX, &tileY);
        printf( "%2d -> %2d : %2d x%2d %2d %s\n",
                x,
                tileX, outputx[i],
                tileWidth,
                x % tileWidth,
                tileX == outputx[i] ? "" : "xxxxxxxxxxxxxxxxxxx" );
        }
    printf(" yyyyyyyyyyyyyyyyyyyyyyyyy\n");
    int inputy[] =  { 5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6};
    int outputy[] = {-3, -3, -2, -2, -1, -1,  0,  0,  1,  1,  2,  2};
    length = sizeof(inputy) / sizeof(int);
    for( int i = 0; i < length; i++ ) {
        int y = inputy[i];
        int tileX;
        int tileY;
        toTileCoord( 0, y, tileWidth, &tileX, &tileY);
        printf( "%2d -> %2d : %2d x%2d %2d %s\n",
                y,
                tileY, outputy[i],
                tileWidth,
                y % tileWidth,
                tileY == outputy[i] ? "" : "xxxxxxxxxxxxxxxxxxx" );
        }
    }

void testCoordTranslation4() {
    int tileWidth = 4;
    printf( "tileWidth %d\n", tileWidth );

    int length;
    printf(" xxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    int inputx[] =  { 5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6};
    int outputx[] = { 1,  1,  0,  0,  0,  0, -1, -1, -1, -1, -2, -2};
    length = sizeof(inputx) / sizeof(int);
    for( int i = 0; i < length; i++ ) {
        int x = inputx[i];
        int tileX;
        int tileY;
        toTileCoord( x, 0, tileWidth, &tileX, &tileY);
        printf( "%2d -> %2d : %2d x%2d %2d %s\n",
                x,
                tileX, outputx[i],
                tileWidth,
                x % tileWidth,
                tileX == outputx[i] ? "" : "xxxxxxxxxxxxxxxxxxx" );
        }
    printf(" yyyyyyyyyyyyyyyyyyyyyyyyy\n");
    int inputy[] =  { 5,  4,  3,  2,  1,  0, -1, -2, -3, -4, -5, -6};
    int outputy[] = {-2, -2, -1, -1, -1, -1,  0,  0,  0,  0,  1,  1};
    length = sizeof(inputy) / sizeof(int);
    for( int i = 0; i < length; i++ ) {
        int y = inputy[i];
        int tileX;
        int tileY;
        toTileCoord( 0, y, tileWidth, &tileX, &tileY);
        printf( "%2d -> %2d : %2d x%2d %2d %s\n",
                y,
                tileY, outputy[i],
                tileWidth,
                y % tileWidth,
                tileY == outputy[i] ? "" : "xxxxxxxxxxxxxxxxxxx" );
        }
    }

// two ints to an 8-byte key
void intPairToKey( int inX, int inY, unsigned char *outKey ) {
    for( int i=0; i<4; i++ ) {
        int offset = i * 8;
        outKey[i] = ( inX >> offset ) & 0xFF;
        outKey[i+4] = ( inY >> offset ) & 0xFF;
        }    
    }

DB db;
unsigned char key[16];
unsigned char value[4];

const char *testFile = "../../../Documents/ohol-family-trees/cache/lifeLog_bigserver2.onehouronelife.com/2019_05May_29_Wednesday.txt";


int main() {
    const char* server = "bigserver2";
    int zoom = 27;

    int tileWidth = pow(2, (32-zoom));
    int around = 500 / tileWidth;

    printf( "%s %d x%d a%d\n", server, zoom, tileWidth, around );

    unsigned char key[8];
    unsigned char value[4];

    //testCoordTranslation2();
    //testCoordTranslation4();

    intToValue( 1, value );

    double startTime = Time::getCurrentTime();

    int error = DB_open( &db, 
                             "tiles.db", 
                             KISSDB_OPEN_MODE_RWCREAT,
                             80000,
                             8, // two ints,  x, y
                             4 // one int
                             );

    if( error ) {
        printf( "Failed to open database\n" );
        return 1;
        }

    FILE* input = fopen( testFile, "rb" );

    if( input == NULL ) {
        printf( "Failed to open %s\n", testFile );
        return 1;
        }

    char kind;
    int x;
    int y;
    int result;
    
    char buffer[256] = {0};
    int crazy = 0;

    x = 0;
    y = -3;

    while( !feof( input ) && !ferror( input ) ) {
        result = fscanf( input, "B %*d %*d %*s %*c (%d,%d) %*s pop=%*d chain=%*d\n",
                &x, &y );

        if( result == 2 ) {
            }
        else {
            result = fscanf( input, "D %*d %*d %*s age=%*f %*c (%d,%d) %*s pop=%*d\n",
                    &x, &y );
            }
        if( result != 2 ) {
            printf( "%d\n", result );
            fgets( buffer, 256, input );
            printf( "bad line\n" );
            printf( "%s\n", buffer );
            }
        int tileX;
        int tileY;
        toTileCoord( x, y, tileWidth, &tileX, &tileY );
        for( int dy = -around; dy <= around; dy++ ) {
            for( int dx = -around; dx <= around; dx++ ) {
                intPairToKey( tileX + dx, tileY + dy, key );
                x = valueToInt( key );
                y = valueToInt( &( key[4] ) );
                DB_put( &db, key, value );
            }
        }
        }

    fclose( input );


    DB_Iterator dbi;

    DB_Iterator_init( &db, &dbi );

    while( DB_Iterator_next( &dbi, key, value ) > 0 ) {
        x = valueToInt( key );
        y = valueToInt( &( key[4] ) );
        printf( "%d,%d\n", x, y );
        }

    DB_close( &db );

    printf( "%f sec\n", 
            Time::getCurrentTime() - startTime );

    return 0;
    }
