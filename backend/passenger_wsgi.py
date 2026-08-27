# ==========================================
# CPANEL ENTRY POINT (Phusion Passenger)
#
# In "Setup Python App" set:
#   Application root:          backend  (the folder you uploaded)
#   Application startup file:  passenger_wsgi.py
# ==========================================

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from config.wsgi import application
