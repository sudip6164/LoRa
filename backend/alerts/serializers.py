from rest_framework import serializers

from .models import SOSAlert


class SOSAlertSerializer(serializers.ModelSerializer):

    class Meta:
        model = SOSAlert
        fields = [
            'id',
            'name',
            'latitude',
            'longitude',
            'device_id',
            'packet_count',
            'rssi',
            'snr',
            'raw_message',
            'status',
            'received_at',
        ]
        read_only_fields = ['status', 'received_at']


class SOSStatusSerializer(serializers.ModelSerializer):

    class Meta:
        model = SOSAlert
        fields = ['id', 'status']
