from django.db import models

class SensorData(models.Model):
    device_id = models.CharField(max_length=100)
    temperature = models.FloatField(null=True, blank=True)
    humidity = models.FloatField(null=True, blank=True)
    gaz = models.FloatField(null=True, blank=True)
    pressure = models.FloatField(null=True, blank=True)
    raw_payload = models.JSONField()
    received_at = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f"{self.device_id} - {self.received_at}"