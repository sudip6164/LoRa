"""
Django settings for the LoRa SOS backend.

EDIT ONLY THE "EDIT THESE" SECTION BELOW BEFORE DEPLOYING.
Everything else can stay as it is.
"""

from pathlib import Path

import os

# Use PyMySQL as the MySQL driver (pure-Python, installs cleanly on cPanel
# shared hosting). Falls back silently if PyMySQL is not installed (local dev
# can stay on SQLite).
try:
    import pymysql
    pymysql.install_as_MySQLdb()
except ImportError:
    pass

BASE_DIR = Path(__file__).resolve().parent.parent


# ==========================================
# EDIT THESE BEFORE DEPLOYING
# ==========================================
# In production (cPanel) set these as environment variables so secrets are
# never committed to source control. The values below are fallbacks for local
# development only and should be overridden via env vars on the server.

# Long random secret string used by Django.
SECRET_KEY = os.getenv('DJANGO_SECRET_KEY', 'GjP(O5ku_9^HV0FoXA+@XDau-GuKmlar5jlt=bKmchjR#cG-=*')

# True while testing on your PC. Must be False on cPanel.
DEBUG = os.getenv('DJANGO_DEBUG', 'True').lower() in ('1', 'true', 'yes', 'on')

# On cPanel replace with your real domain. Local dev also allows localhost.
LOCAL_IPS = ['127.0.0.1', 'localhost']
ALLOWED_HOSTS = ['backend.nirvix.com'] + (LOCAL_IPS if DEBUG else [])

# The ESP32 must send this in the "X-API-Key" header.
SOS_API_KEY = os.getenv('SOS_API_KEY', 'wRJLAb4lVXwWRGEWZiMA2xF4v2cu71dk')

# Timezone shown on the dashboard
TIME_ZONE = 'Asia/Kathmandu'

# Comma-separated list of frontend origins allowed to call the API (Next.js on
# Vercel), e.g. "https://my-app.vercel.app,https://my-app-git-dev.vercel.app".
# If left empty, only same-origin requests work.
CORS_ALLOWED_ORIGINS = [
    o.strip() for o in os.getenv('CORS_ALLOWED_ORIGINS', '').split(',') if o.strip()
]

# Set to "True" to allow ANY origin (dev only - do not use in production).
CORS_ALLOW_ALL_ORIGINS = os.getenv('CORS_ALLOW_ALL_ORIGINS', 'False').lower() in (
    '1', 'true', 'yes', 'on'
)


# ==========================================
# DATABASE (set these on cPanel to use MySQL)
# ==========================================
# Leave DB_ENGINE unset (or any value other than "mysql") to keep using the
# local SQLite file. On cPanel create a MySQL DB + user, grant all privileges,
# then set these env vars:
#   DB_ENGINE=mysql
#   DB_NAME=<your_mysql_database>
#   DB_USER=<your_mysql_user>
#   DB_PASSWORD=<your_mysql_password>
#   DB_HOST=<usually "localhost">
#   DB_PORT=3306
DB_ENGINE = os.getenv('DB_ENGINE', 'sqlite')
DB_NAME = os.getenv('DB_NAME', str(BASE_DIR / 'db.sqlite3'))
DB_USER = os.getenv('DB_USER', '')
DB_PASSWORD = os.getenv('DB_PASSWORD', '')
DB_HOST = os.getenv('DB_HOST', 'localhost')
DB_PORT = os.getenv('DB_PORT', '3306')


# ==========================================
# CORE SETTINGS (no changes needed)
# ==========================================

INSTALLED_APPS = [
    'django.contrib.admin',
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.messages',
    'django.contrib.staticfiles',
    'rest_framework',
    'corsheaders',
    'alerts',
]

MIDDLEWARE = [
    'corsheaders.middleware.CorsMiddleware',
    'django.middleware.security.SecurityMiddleware',
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
    'django.contrib.messages.middleware.MessageMiddleware',
    'django.middleware.clickjacking.XFrameOptionsMiddleware',
]

ROOT_URLCONF = 'config.urls'

TEMPLATES = [
    {
        'BACKEND': 'django.template.backends.django.DjangoTemplates',
        'DIRS': [],
        'APP_DIRS': True,
        'OPTIONS': {
            'context_processors': [
                'django.template.context_processors.debug',
                'django.template.context_processors.request',
                'django.contrib.auth.context_processors.auth',
                'django.contrib.messages.context_processors.messages',
            ],
        },
    },
]

WSGI_APPLICATION = 'config.wsgi.application'

DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.sqlite3',
        'NAME': BASE_DIR / 'db.sqlite3',
    }
}

if DB_ENGINE == 'mysql':
    DATABASES['default'] = {
        'ENGINE': 'django.db.backends.mysql',
        'NAME': DB_NAME,
        'USER': DB_USER,
        'PASSWORD': DB_PASSWORD,
        'HOST': DB_HOST,
        'PORT': DB_PORT,
        'OPTIONS': {'charset': 'utf8mb4'},
    }

AUTH_PASSWORD_VALIDATORS = [
    {'NAME': 'django.contrib.auth.password_validation.UserAttributeSimilarityValidator'},
    {'NAME': 'django.contrib.auth.password_validation.MinimumLengthValidator'},
    {'NAME': 'django.contrib.auth.password_validation.CommonPasswordValidator'},
    {'NAME': 'django.contrib.auth.password_validation.NumericPasswordValidator'},
]

LANGUAGE_CODE = 'en-us'

USE_I18N = True

USE_TZ = True

STATIC_URL = 'static/'
STATIC_ROOT = BASE_DIR / 'staticfiles'

DEFAULT_AUTO_FIELD = 'django.db.models.BigAutoField'
