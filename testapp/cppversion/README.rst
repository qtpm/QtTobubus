Cppversion
=================================

Description about your awesome package.

Example
--------------

.. code-block:: cpp

   #include <iostream>
   #include <cppversion/cppversion.h>

   int main() {
       auto obj = Cppversion();
       std::cout << obj.awesomeMethod() << std::endl;
   }

Reference
--------------

Write reference here

Develop
--------------

``qtpm`` command helps your development.

.. code-block:: bash

   $ git clone git@github.com:shibukawa.yoshiki/cppversion.git
   $ cd cppversion

   # Run the following command once if this package has required packages
   # -u updates modules they are already downloaded
   $ qtpm get [-u]

   # Build and run test
   $ qtpm test

   # Try build
   $ qtpm build


Folder Structure and Naming Convention
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

   + src/           // Header files under this folder will be published.
   |  + private/    // Files under this folder will hide from outer modules.
   + resources/     // .qrc files are linked to application.
   + test/          // Each _test.cpp file is built into executable and launched by qtpm test.
   |                // Use `qtpm add test`` to add test case.
   |                // Other .cpp files are used as utility code. They are linked to test modules.
   + doc/           // (not working yet) QDoc setting file and extra image files and so on.
   + html/          // (not working yet) HTML output of QDoc.
   + vendor/        // External libraries (source and binary)
   + build/         // Work folder.
   + CMakeExtra.txt // This file is included from created CMakeLists.txt.
                    // You can add any extra build options here.

Author
--------------

* shibukawa.yoshiki

License
--------------

The MIT License (MIT)

History
--------------

* Write package history here