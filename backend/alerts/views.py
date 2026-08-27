from django.conf import settings
from django.http import JsonResponse
from django.shortcuts import get_object_or_404, render
from django.views.decorators.csrf import csrf_exempt
from django.views.decorators.http import require_POST
from rest_framework import generics
from rest_framework.permissions import BasePermission

from .models import SOSAlert
from .serializers import SOSAlertSerializer, SOSStatusSerializer


class HasAPIKey(BasePermission):
    """Allows the request only when the correct X-API-Key header is sent."""

    message = 'Invalid or missing X-API-Key header.'

    def has_permission(self, request, view):
        return request.headers.get('X-API-Key', '') == settings.SOS_API_KEY


class SOSListCreate(generics.ListCreateAPIView):
    """GET: list alerts (dashboard). POST: create alert (ESP32)."""

    queryset = SOSAlert.objects.all()
    serializer_class = SOSAlertSerializer

    def get_permissions(self):
        if self.request.method == 'POST':
            return [HasAPIKey()]
        return []


class SOSDetail(generics.RetrieveUpdateAPIView):
    """GET: one alert. PATCH: update status (needs API key)."""

    queryset = SOSAlert.objects.all()

    def get_permissions(self):
        if self.request.method in ('PUT', 'PATCH'):
            return [HasAPIKey()]
        return []

    def get_serializer_class(self):
        if self.request.method in ('PUT', 'PATCH'):
            return SOSStatusSerializer
        return SOSAlertSerializer


def dashboard(request):
    """Simple live dashboard page."""
    return render(request, 'dashboard.html')


@csrf_exempt
@require_POST
def resolve_alert(request, pk):
    """Mark an alert as RESOLVED (used by the Resolve button on dashboard).

    CSRF is exempted because the request is issued from the same-origin
    dashboard via fetch(); without this the middleware would block it with 403.
    For real access control, add a login system and gate this view.
    """
    alert = get_object_or_404(SOSAlert, pk=pk)
    alert.status = 'RESOLVED'
    alert.save(update_fields=['status'])
    return JsonResponse({'ok': True, 'id': alert.pk, 'status': alert.status})
