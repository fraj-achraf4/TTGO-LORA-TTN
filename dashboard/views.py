from django.shortcuts import render
from django.contrib.auth.decorators import login_required
from .models import SensorData

def home(request):
    return render(request, 'home.html')

def sensor_data_list(request):
    data = SensorData.objects.all().order_by('-received_at')  # Dernières données en premier
    return render(request, 'sensor_data_list.html', {'data': data})