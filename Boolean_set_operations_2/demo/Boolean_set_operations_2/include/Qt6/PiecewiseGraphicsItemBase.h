// Copyright (c) 2009  GeometryFactory Sarl (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s) : Efi Fogel <efifogel@gmail.com>
//             Fernando Cacciola <fernando.cacciola@geometryfactory.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>

#ifndef CGAL_QT6_PIECEWISE_GRAPHICS_ITEM_BASE_BEZIER_H
#define CGAL_QT6_PIECEWISE_GRAPHICS_ITEM_BASE_BEZIER_H

#include <boost/optional.hpp>
#include <boost/utility.hpp>

#include <CGAL/function_objects.h>
#include <CGAL/Bbox_2.h>
#include <CGAL/Qt/GraphicsItem.h>
#include <CGAL/Qt/Converter.h>

#include <QPainter>
#include <QBrush>
#include <QPen>

#include "Typedefs.h"

namespace CGAL {
namespace Qt {

/*! The base class of the graphics items that draw curves, boundaries, polygons with holes, and polygon sets.
 */
class Piecewise_graphics_item_base_bezier : public GraphicsItem
{
protected:
  /*! constructs default.
   */
  Piecewise_graphics_item_base_bezier() {}

public:
  /*! recomputes the bounding rectangle.
   */
  void update_bounding_box();

  /*! updates the item after its model changed.
   */
  void modelChanged() {
    update_bounding_box();
    update();
  }

  /*! obtains the bounding rectangle. An unbounded set is painted all over, so its bounding rectangle must cover it
   * all, or else Qt repaints only parts of it.
   */
  QRectF boundingRect() const { return mUnbounded ? unbounded_rect() : mBounding_rect; }

  /*! obtains the bounding rectangle of the curves, e.g., for zooming to fit.
   */
  QRectF curves_rect() const { return mBounding_rect; }

  /*! determines whether the model is unbounded.
   */
  bool is_unbounded() const { return mUnbounded; }

  /*! obtains the area that unbounded sets (e.g., complements) are drawn over, which the application keeps a bit larger
   * than the visible area of the view.
   */
  static QRectF& unbounded_rect() {
    static QRectF s_rect(-320, -210, 640, 420);
    return s_rect;
  }

  /*! paints the model.
   */
  void paint(QPainter* aPainter, const QStyleOptionGraphicsItem* aOption, QWidget* aWidget);

  /*! obtains the brush.
   */
  const QBrush& brush() const { return mBrush; }

  /*! sets the brush.
   */
  void set_brush(const QBrush& aBrush) { mBrush = aBrush; }

  /*! obtains the pen.
   */
  const QPen& pen() const { return mPen; }

  /*! sets the pen.
   */
  void set_pen(const QPen& aPen) { mPen = aPen; }

protected:
  using ToQtConverter = Converter<Simple_cartesian<double>>;

  /*! Accumulates bounding boxes.
   */
  struct Bbox_builder
  {
    /*! extends the box with a given box.
     */
    void add(const Bbox_2& aBbox) {
      if (bbox) bbox = *bbox + aBbox;
      else bbox = aBbox;
    }

    boost::optional<Bbox_2> bbox;
    bool unbounded = false;
  };

  /*! determines whether there is nothing to draw.
   */
  virtual bool is_model_empty() const = 0;

  /*! adds the model to the path.
   */
  virtual void draw_model(QPainterPath& aPath) = 0;

  /*! extends the bounding box with the box of the model.
   */
  virtual void update_bbox(Bbox_builder& aBBoxBuilder) = 0;

protected:
  QRectF mBounding_rect;
  bool mUnbounded = false;
  QBrush mBrush;
  QPen mPen;
};

void Piecewise_graphics_item_base_bezier::paint(QPainter* aPainter, const QStyleOptionGraphicsItem* /* aOption */,
                                                QWidget* /* aWidget */) {
  if (is_model_empty()) return;
  QPainterPath lPath;
  draw_model(lPath);
  aPainter->setPen(mPen);
  aPainter->setBrush(mBrush);
  aPainter->drawPath(lPath);
}

// The bounding box only grows, so that when vertices get removed the maximal bbox gets refreshed in the GraphicsView.
void Piecewise_graphics_item_base_bezier::update_bounding_box() {
  if (is_model_empty()) {
    if (mUnbounded) {
      prepareGeometryChange();
      mUnbounded = false;
    }
    return;
  }
  prepareGeometryChange();
  Bbox_builder lBBoxBuilder;
  update_bbox(lBBoxBuilder);
  mUnbounded = lBBoxBuilder.unbounded;
  if (lBBoxBuilder.bbox) {
    ToQtConverter to_Qt;
    mBounding_rect = to_Qt(*lBBoxBuilder.bbox);
  }
}

} // namespace Qt
} // namespace CGAL

#endif
