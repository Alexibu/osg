/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/GLExtensions>
#include <osg/ref_ptr>
#include <osg/Image>
#include <osg/State>
#include <osg/TextureCubeMap>

#include <osg/GLU>


using namespace osg;


// include/osg/TextureCubeMap defines GL_TEXTURE_CUBE_MAP to be
// 0x8513 which is the same as GL_TEXTURE_CUBE_MAP_ARB & _EXT.
// assume its the same as what OpenGL 1.3 defines.

#ifndef GL_ARB_texture_cube_map
#define GL_ARB_texture_cube_map 1
#define GL_TEXTURE_CUBE_MAP_ARB             0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB     0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB  0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB  0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB  0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB  0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB  0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB  0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB       0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB    0x851C
#endif


#ifndef GL_EXT_texture_cube_map
#define GL_EXT_texture_cube_map 1
#define GL_TEXTURE_CUBE_MAP_EXT             0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C
#endif


#ifdef GL_ARB_texture_cube_map
#  define CUBE_MAP_POSITIVE_X   GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
#  define CUBE_MAP_NEGATIVE_X   GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB
#  define CUBE_MAP_POSITIVE_Y   GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB
#  define CUBE_MAP_NEGATIVE_Y   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
#  define CUBE_MAP_POSITIVE_Z   GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB
#  define CUBE_MAP_NEGATIVE_Z   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
#elif GL_EXT_texture_cube_map
#  define CUBE_MAP_POSITIVE_X   GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT
#  define CUBE_MAP_NEGATIVE_X   GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT
#  define CUBE_MAP_POSITIVE_Y   GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT
#  define CUBE_MAP_NEGATIVE_Y   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT
#  define CUBE_MAP_POSITIVE_Z   GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT
#  define CUBE_MAP_NEGATIVE_Z   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT
#endif


# if GL_EXT_texture_cube_map || GL_ARB_texture_cube_map
static GLenum faceTarget[6] =
{
    CUBE_MAP_POSITIVE_X,
    CUBE_MAP_NEGATIVE_X,
    CUBE_MAP_POSITIVE_Y,
    CUBE_MAP_NEGATIVE_Y,
    CUBE_MAP_POSITIVE_Z,
    CUBE_MAP_NEGATIVE_Z
};
#endif


TextureCubeMap::TextureCubeMap():
            _textureWidth(0),
            _textureHeight(0),
            _numMipmapLevels(0)
{
    setUseHardwareMipMapGeneration(false);
}

TextureCubeMap::TextureCubeMap(const TextureCubeMap& text,const CopyOp& copyop):
            Texture(text,copyop),
            _textureWidth(text._textureWidth),
            _textureHeight(text._textureHeight),
            _numMipmapLevels(text._numMipmapLevels),
            _subloadCallback(text._subloadCallback)
{
    _images[0] = copyop(text._images[0].get());
    _images[1] = copyop(text._images[1].get());
    _images[2] = copyop(text._images[2].get());
    _images[3] = copyop(text._images[3].get());
    _images[4] = copyop(text._images[4].get());
    _images[5] = copyop(text._images[5].get());

    _modifiedCount[0].setAllElementsTo(0);
    _modifiedCount[1].setAllElementsTo(0);
    _modifiedCount[2].setAllElementsTo(0);
    _modifiedCount[3].setAllElementsTo(0);
    _modifiedCount[4].setAllElementsTo(0);
    _modifiedCount[5].setAllElementsTo(0);

}    


TextureCubeMap::~TextureCubeMap()
{
}


int TextureCubeMap::compare(const StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Paramter macro's below.
    COMPARE_StateAttribute_Types(TextureCubeMap,sa)

    for (int n=0; n<6; n++)
    {
        if (_images[n]!=rhs._images[n]) // smart pointer comparison.
        {
            if (_images[n].valid())
            {
                if (rhs._images[n].valid())
                {
                    int result = _images[n]->compare(*rhs._images[n]);
                    if (result!=0) return result;
                }
                else
                {
                    return 1; // valid lhs._image is greater than null. 
                }
            }
            else if (rhs._images[n].valid()) 
            {
                return -1; // valid rhs._image is greater than null. 
            }
        }
    }

    int result = compareTexture(rhs);
    if (result!=0) return result;

    // compare each paramter in turn against the rhs.
    COMPARE_StateAttribute_Parameter(_textureWidth)
    COMPARE_StateAttribute_Parameter(_textureHeight)
    COMPARE_StateAttribute_Parameter(_subloadCallback)

    return 0; // passed all the above comparison macro's, must be equal.
}


void TextureCubeMap::setImage( unsigned int face, Image* image)
{
    _images[face] = image;
    _modifiedCount[face].setAllElementsTo(0);
}

Image* TextureCubeMap::getImage(unsigned int face)
{
    return _images[face].get();
}

const Image* TextureCubeMap::getImage(unsigned int face) const
{
    return _images[face].get();
}

bool TextureCubeMap::imagesValid() const
{
    for (int n=0; n<6; n++)
    {
        if (!_images[n].valid() || !_images[n]->data())
            return false;
    }
    return true;
}

void TextureCubeMap::computeInternalFormat() const
{
    if (imagesValid()) computeInternalFormatWithImage(*_images[0]); 
}

void TextureCubeMap::apply(State& state) const
{
    // get the contextID (user defined ID of 0 upwards) for the 
    // current OpenGL context.
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = getExtensions(contextID,true);

    if (!extensions->isCubeMapSupported())
        return;

    // get the texture object for the current contextID.
    TextureObject* textureObject = getTextureObject(contextID);

    if (textureObject != 0)
    {
        textureObject->bind();

        if (getTextureParameterDirty(state.getContextID())) applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        if (_subloadCallback.valid())
        {
            _subloadCallback->subload(*this,state);
        }
        else
        {
            for (int n=0; n<6; n++)
            {
                const osg::Image* image = _images[n].get();
                if (image && getModifiedCount((Face)n,contextID) != image->getModifiedCount())
                {
                    applyTexImage2D_subload( state, faceTarget[n], _images[n].get(), _textureWidth, _textureHeight, _internalFormat, _numMipmapLevels);
                    getModifiedCount((Face)n,contextID) = image->getModifiedCount();
                }
            }
        }

    }
    else if (_subloadCallback.valid())
    {
        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(contextID,GL_TEXTURE_CUBE_MAP);

        textureObject->bind();

        applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        _subloadCallback->load(*this,state);

        // in theory the following line is redundent, but in practice
        // have found that the first frame drawn doesn't apply the textures
        // unless a second bind is called?!!
        // perhaps it is the first glBind which is not required...
        //glBindTexture( GL_TEXTURE_CUBE_MAP, handle );

    }
    else if (imagesValid())
    {

        // compute the internal texture format, this set the _internalFormat to an appropriate value.
        computeInternalFormat();

        // compute the dimensions of the texture.
        computeRequiredTextureDimensions(state,*_images[0],_textureWidth, _textureHeight, _numMipmapLevels);

        _textureObjectBuffer[contextID] = textureObject = generateTextureObject(
                contextID,GL_TEXTURE_CUBE_MAP,_numMipmapLevels,_internalFormat,_textureWidth,_textureHeight,1,0);
        
        textureObject->bind();

        applyTexParameters(GL_TEXTURE_CUBE_MAP,state);

        for (int n=0; n<6; n++)
        {
            const osg::Image* image = _images[n].get();
            if (image)
            {
                if (textureObject->isAllocated())
                {
                    applyTexImage2D_subload( state, faceTarget[n], image, _textureWidth, _textureHeight, _internalFormat, _numMipmapLevels);
                }
                else
                {
                    applyTexImage2D_load( state, faceTarget[n], image, _textureWidth, _textureHeight, _numMipmapLevels);
                }
                getModifiedCount((Face)n,contextID) = image->getModifiedCount();
            }


        }

        if (_unrefImageDataAfterApply && areAllTextureObjectsLoaded())
        {
            TextureCubeMap* non_const_this = const_cast<TextureCubeMap*>(this);
            for (int n=0; n<6; n++)
            {                
                if (_images[n].valid() && _images[n]->getDataVariance()==STATIC)
                {
                    non_const_this->_images[n] = 0;
                }
            }
        }
        
    }
    else
    {
        glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    }
}

typedef buffered_value< ref_ptr<TextureCubeMap::Extensions> > BufferedExtensions;
static BufferedExtensions s_extensions;

TextureCubeMap::Extensions* TextureCubeMap::getExtensions(unsigned int contextID,bool createIfNotInitalized)
{
    if (!s_extensions[contextID] && createIfNotInitalized) s_extensions[contextID] = new Extensions;
    return s_extensions[contextID].get();
}

void TextureCubeMap::setExtensions(unsigned int contextID,Extensions* extensions)
{
    s_extensions[contextID] = extensions;
}

TextureCubeMap::Extensions::Extensions()
{
    setupGLExtenions();
}

TextureCubeMap::Extensions::Extensions(const Extensions& rhs):
    Referenced()
{
    _isCubeMapSupported = rhs._isCubeMapSupported;
}

void TextureCubeMap::Extensions::lowestCommonDenominator(const Extensions& rhs)
{
    if (!rhs._isCubeMapSupported) _isCubeMapSupported = false;
}

void TextureCubeMap::Extensions::setupGLExtenions()
{
    _isCubeMapSupported = isGLExtensionSupported("GL_ARB_texture_cube_map") ||
                          isGLExtensionSupported("GL_EXT_texture_cube_map") ||
                          strncmp((const char*)glGetString(GL_VERSION),"1.3",3)>=0;;
}
