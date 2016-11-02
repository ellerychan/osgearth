/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2014 Pelican Mapping
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include "LayerDrawable"

using namespace osgEarth::Drivers::RexTerrainEngine;

#undef  LC
#define LC "[LayerDrawable] "


LayerDrawable::LayerDrawable() :
_order(0),
_layer(0L),
_clearOsgState(false)
{
    setDataVariance(DYNAMIC);
    setUseDisplayList(false);
    setUseVertexBufferObjects(true);
}

void
LayerDrawable::drawImplementation(osg::RenderInfo& ri) const
{    
    DrawState& ds = *_drawState;

    // Make sure the draw state is up to date:
    ds.refresh(ri);

    if (ds._layerOrderUL >= 0)
    {
        ds._ext->glUniform1i(ds._layerOrderUL, (GLint)_order);
    }

    if (_layer)
    {
        if (ds._layerUidUL >= 0)
            ds._ext->glUniform1i(ds._layerUidUL,      (GLint)_layer->getUID());
        if (ds._layerOpacityUL >= 0 && _imageLayer)
            ds._ext->glUniform1f(ds._layerOpacityUL,  (GLfloat)_imageLayer->getOpacity());
        if (ds._layerMinRangeUL >= 0 && _imageLayer)
            ds._ext->glUniform1f(ds._layerMinRangeUL, (GLfloat)_imageLayer->getMinVisibleRange());
        if (ds._layerMaxRangeUL >= 0 && _imageLayer)
            ds._ext->glUniform1f(ds._layerMaxRangeUL, (GLfloat)_imageLayer->getMaxVisibleRange());
    }
    else
    {
        if (ds._layerUidUL >= 0)
            ds._ext->glUniform1i(ds._layerUidUL,      (GLint)-1);
        if (ds._layerOpacityUL >= 0)
            ds._ext->glUniform1f(ds._layerOpacityUL,  (GLfloat)1.0f);
        if (ds._layerMinRangeUL >= 0)
            ds._ext->glUniform1f(ds._layerMinRangeUL, (GLfloat)0.0f);
        if (ds._layerMaxRangeUL >= 0)
            ds._ext->glUniform1f(ds._layerMaxRangeUL, (GLfloat)FLT_MAX);
    }

    for (DrawTileCommands::const_iterator tile = _tiles.begin(); tile != _tiles.end(); ++tile)
    {
        tile->draw(ri, ds, 0L);
    }

    // If set, dirty all OSG state to prevent any leakage - this is sometimes
    // necessary when doing custom OpenGL within a Drawable.
    if (_clearOsgState)
    {
        ri.getState()->dirtyAllAttributes();
        ri.getState()->dirtyAllModes();
        ri.getState()->dirtyAllVertexArrays();
        
        // unbind local buffers when finished.
        ds._ext->glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
        ds._ext->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);

        ri.getState()->apply();
    }
}