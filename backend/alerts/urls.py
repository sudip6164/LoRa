from django.urls import path
from django.views.generic import RedirectView

from . import views

urlpatterns = [
    path('', RedirectView.as_view(url='/dashboard/', permanent=False)),
    path('dashboard/', views.dashboard, name='dashboard'),
    path('dashboard/resolve/<int:pk>/', views.resolve_alert, name='resolve-alert'),
    path('api/sos/', views.SOSListCreate.as_view(), name='sos-list-create'),
    path('api/sos/<int:pk>/', views.SOSDetail.as_view(), name='sos-detail'),
]
