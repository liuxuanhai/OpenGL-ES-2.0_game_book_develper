/*

Book:      	Game and Graphics Programming for iOS and Android with OpenGL(R) ES 2.0
Author:    	Romain Marucchi-Foino
ISBN-10: 	1119975913
ISBN-13: 	978-1119975915
Publisher: 	John Wiley & Sons	

Copyright (C) 2011 Romain Marucchi-Foino

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of
this software. Permission is granted to anyone who either own or purchase a copy of
the book specified above, to use this software for any purpose, including commercial
applications subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
you wrote the original software. If you use this software in a product, an acknowledgment
in the product would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented
as being the original software.

3. This notice may not be removed or altered from any source distribution.

*/

#include "templateApp.h"

#define OBJ_FILE ( char * )"Scene.obj"

OBJ *obj = NULL;

TEMPLATEAPP templateApp = { templateAppInit,
							templateAppDraw };

OBJMESH *objmesh = NULL;

int viewport_matrix[ 4 ];

LIGHT *light = NULL;


void program_bind_attrib_location( void *ptr ) {

	PROGRAM *program = ( PROGRAM * )ptr;

	glBindAttribLocation( program->pid, 0, "POSITION"  );
	glBindAttribLocation( program->pid, 1, "NORMAL"    );
	glBindAttribLocation( program->pid, 2, "TEXCOORD0" );
	glBindAttribLocation( program->pid, 3, "TANGENT0"  );
}


void program_draw( void *ptr )
{
	unsigned int i = 0;
	
	PROGRAM *program = ( PROGRAM * )ptr;
	
	while( i != program->uniform_count )
	{
		if( program->uniform_array[ i ].constant ) 
		{
			++i;
			continue;
		}
	
		else if( !strcmp( program->uniform_array[ i ].name, "MODELVIEWPROJECTIONMATRIX" ) )
		{
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_modelview_projection_matrix() );			
		}
		
		else if( !strcmp( program->uniform_array[ i ].name, "DIFFUSE" ) )
		{
			glUniform1i( program->uniform_array[ i ].location,
						 1 );
			
			program->uniform_array[ i ].constant = 1;
		}

		else if( !strcmp( program->uniform_array[ i ].name, "BUMP" ) )
		{
			glUniform1i( program->uniform_array[ i ].location,
						 4 );
						 
			program->uniform_array[ i ].constant = 1;
		}

		// Matrix Data
		else if( !strcmp( program->uniform_array[ i ].name, "MODELVIEWMATRIX" ) )
		{
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_modelview_matrix() );			
		}

		else if( !strcmp( program->uniform_array[ i ].name, "PROJECTIONMATRIX" ) )
		{
			glUniformMatrix4fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_projection_matrix() );
			
			program->uniform_array[ i ].constant = 1;
		}

		else if( !strcmp( program->uniform_array[ i ].name, "NORMALMATRIX" ) )
		{
			glUniformMatrix3fv( program->uniform_array[ i ].location,
								1,
								GL_FALSE,
								( float * )GFX_get_normal_matrix() );			
		}


		// Material Data
		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.ambient" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmesh->current_material->ambient );
						 
			program->uniform_array[ i ].constant = 1;
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.diffuse" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmesh->current_material->diffuse );
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.specular" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&objmesh->current_material->specular );	
		}		

		else if( !strcmp( program->uniform_array[ i ].name, "MATERIAL.shininess" ) )
		{
			glUniform1f( program->uniform_array[ i ].location,
						 objmesh->current_material->specular_exponent * 0.128f );

			program->uniform_array[ i ].constant = 1;
		}
		

		// Lamp Data
		else if( !strcmp( program->uniform_array[ i ].name, "LIGHT_FS.color" ) )
		{
			glUniform4fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&light->color );

			program->uniform_array[ i ].constant = 1;						  
		}

		else if( !strcmp( program->uniform_array[ i ].name, "LIGHT_VS.position" ) )
		{
			vec4 position;
		
			LIGHT_get_position_in_eye_space( light,
											 &gfx.modelview_matrix[ gfx.modelview_matrix_index - 1 ], 
											 &position );
			
			glUniform3fv( program->uniform_array[ i ].location,
						  1,
						  ( float * )&position );

			program->uniform_array[ i ].constant = 1;
		}

		++i;
	}	
}


void templateAppInit( int width, int height ) {

	atexit( templateAppExit );

	GFX_start();

	glViewport( 0.0f, 0.0f, width, height );
	
	glGetIntegerv( GL_VIEWPORT, viewport_matrix );

	obj = OBJ_load( OBJ_FILE, 1 );

	unsigned int i = 0;

	while( i != obj->n_objmesh ) {
		
		OBJ_optimize_mesh( obj, i, 128 );

		OBJ_build_mesh2( obj, i );

		OBJ_free_mesh_vertex_data( obj, i ); 

		++i;
	}
	

	i = 0;
	while( i != obj->n_texture ) { 

		OBJ_build_texture( obj,
						   i,
						   obj->texture_path,
						   TEXTURE_MIPMAP | TEXTURE_16_BITS,
						   TEXTURE_FILTER_2X,
						   0.0f );
		++i;
	}


	i = 0;
	while( i != obj->n_program ) { 
		
		OBJ_build_program( obj,
						   i,
						   program_bind_attrib_location,
						   program_draw,
						   1,
						   obj->program_path );
		++i;
	}


	i = 0;
	while( i != obj->n_objmaterial ) { 

		OBJ_build_material( obj, i, NULL );
		++i;
	}
	
	
	vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	vec3 position = { 0.0f, 0.0f, 5.0f };	
	
	light = LIGHT_create_point( ( char * )"point", &color, &position );
	
	OBJ_get_mesh( obj, "sphere", 0 )->objtrianglelist[ 0 ].mode = GL_POINTS;
}


void templateAppDraw( void ) {

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	GFX_set_matrix_mode( PROJECTION_MATRIX );
	GFX_load_identity();
	
	GFX_set_perspective( 45.0f,
						 ( float )viewport_matrix[ 2 ] / ( float )viewport_matrix[ 3 ],
						 0.1f,
						 100.0f,
						 -90.0f );
	
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

	GFX_set_matrix_mode( MODELVIEW_MATRIX );
	GFX_load_identity();

	GFX_translate( 14.0f, -12.0f, 7.0f );

	GFX_rotate( 48.5f, 0.0f, 0.0f, 1.0f );

	GFX_rotate( 72.0, 1.0f, 0.0f, 0.0f );
	
	mat4_invert( GFX_get_modelview_matrix() );


	unsigned int i = 0;

	while( i != obj->n_objmesh ) {

		objmesh = &obj->objmesh[ i ];

		GFX_push_matrix();

		GFX_translate( objmesh->location.x,
					   objmesh->location.y,
					   objmesh->location.z );

		if( strstr( objmesh->name, "sphere" ) )
		{
			glEnable( GL_BLEND );
			glDepthMask( GL_FALSE );
			
			glBlendEquation( GL_FUNC_ADD );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );

			objmesh->rotation.x += 0.5f;
			objmesh->rotation.y += 0.5f;
			objmesh->rotation.z += 0.5f;

			GFX_rotate( objmesh->rotation.z, 0.0f, 0.0f, 1.0f );
			GFX_rotate( objmesh->rotation.y, 0.0f, 1.0f, 0.0f );
			GFX_rotate( objmesh->rotation.x, 1.0f, 0.0f, 0.0f );
			
			OBJ_draw_mesh( obj, i );

			glDisable( GL_BLEND );
			glDepthMask( GL_TRUE );
		}
		else OBJ_draw_mesh( obj, i );


		GFX_pop_matrix();
		
		++i;
	}
}


void templateAppExit( void ) {

	light = LIGHT_free( light );

	OBJ_free( obj );
}
