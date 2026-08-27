"""
Django settings for the LoRa SOS backend.

EDIT ONLY THE "EDIT THESE" SECTION BELOW BEFORE DEPLOYING.
Everything else can stay as it is.
"""

from pathlib import Path

import os

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
    'alerts',
]

MIDDLEWARE = [
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
