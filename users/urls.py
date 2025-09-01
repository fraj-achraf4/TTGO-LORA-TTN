# users/urls.py
from django.urls import path
from .views import SignUpView  # Importez votre SignUpView depuis views.py

urlpatterns = [
    # L'URL pour la page d'inscription
    path('signup/', SignUpView.as_view(), name='signup'),
]