// Copyright (c) 2012  Tel-Aviv University (Israel).
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
//             Kabir Kedia <kabirkedia0111@gmail.com>
//             Ronnie Gandhi <ronniegandhi19999@gmail.com>
//             Apurva Bhatt <response2apurva@gmail.com>

/*! \file Boolean_set_operations_2.cpp
 * The 2D regularized Boolean set-operations demo: runs the main window.
 */

#include <exception>
#include <string>

#include <QApplication>

#include <CGAL/Qt/resources.h>

#include "Main_window.h"
#include "Error_handling.h"

/*! runs the demo.
 */
int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  app.setOrganizationDomain("geometryfactory.com");
  app.setOrganizationName("GeometryFactory");
  app.setApplicationName("Boolean_operations_2 demo");
  CGAL_QT_INIT_RESOURCES;
  try {
    MainWindow w;
    w.show();
    return app.exec();
  }
  catch (const std::exception& e) {
    std::string s = e.what();
    show_error("Exception thrown during run of the program:\n" + s);
  }
}
