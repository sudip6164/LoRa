from django.contrib import admin

from .models import SOSAlert


@admin.register(SOSAlert)
class SOSAlertAdmin(admin.ModelAdmin):
    list_display = (
        'id',
        'name',
        'device_id',
        'latitude',
        'longitude',
        'packet_count',
        'rssi',
        'status',
        'received_at',
    )
    list_filter = ('status',)
    search_fields = ('name', 'device_id')
    readonly_fields = (
        'name', 'latitude', 'longitude', 'device_id', 'packet_count',
        'rssi', 'snr', 'raw_message', 'received_at',
    )
