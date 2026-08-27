from django.db import migrations, models


class Migration(migrations.Migration):

    initial = True

    dependencies = []

    operations = [
        migrations.CreateModel(
            name='SOSAlert',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('name', models.CharField(max_length=100)),
                ('latitude', models.FloatField(default=0.0)),
                ('longitude', models.FloatField(default=0.0)),
                ('device_id', models.CharField(blank=True, max_length=20)),
                ('packet_count', models.IntegerField(default=0)),
                ('rssi', models.IntegerField(blank=True, null=True)),
                ('snr', models.FloatField(blank=True, null=True)),
                ('raw_message', models.TextField(blank=True)),
                ('status', models.CharField(choices=[('PENDING', 'Pending'), ('RESOLVED', 'Resolved')], default='PENDING', max_length=10)),
                ('received_at', models.DateTimeField(auto_now_add=True)),
            ],
            options={
                'ordering': ['-received_at'],
            },
        ),
    ]
