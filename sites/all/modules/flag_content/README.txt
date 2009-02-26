$Id: README.txt,v 1.2 2006/09/13 20:51:02 kbahey Exp $

Copyright 2006 http://2bits.com

Description
-----------
This module allows users to flag nodes as offensive for the adminstrator
to review.

The admin can specify which type of nodes are allowed to be flagged.

An email is sent to the administrator to notify them who flagged what. The
flagged nodes are also queued for the administrator to review. They can edit,
delete or unflag the node.

Note: If  you grant anonymous users the permission to flag content, there may
be false positives because of crawlers.

Installation
------------
To install this module, Upload or copy the the entire flag_content directory and 
all its contents to your modules directory.

Configuration
-------------
To enable this module do the following:

1. Go to Administer -> Modules, and enable flag_content.

2. Go to Administer -> Access Control and enable for the roles you want.

3. Go to Administer -> Settings -> flag_content and configure the 
   node types that are allowed to be flagged.

Wishlist
--------
- Extend the module to handle flagging of comments as well.

- Handle multiple configurable flagging reasons (e.g. offensive, spam,
  fraud, off-topic, ...etc.)

Bugs/Features/Patches:
----------------------
If you want to report bugs, feature requests, or submit a patch, please do so
at the project page on the Drupal web site.

Author
------

Sponsored by http://socialsignal.com for http://changeeverything.ca

Khalid Baheyeldin (http://baheyeldin.com/khalid and http://2bits.com)

If you use this module, find it useful, and want to send the author
a thank you note, then use the Feedback/Contact page at the URL above.

The author can also be contacted for paid customizations of this
and other modules.
