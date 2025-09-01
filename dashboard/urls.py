# dashboard/urls.py
from django.urls import path
from .views import home, sensor_data_list


urlpatterns = [
    path('', home, name='home'),
    path('dashboard/', sensor_data_list, name='dashboard'), 
]