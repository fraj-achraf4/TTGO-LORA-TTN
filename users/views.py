# users/views.py
from django.contrib.auth.forms import UserCreationForm
from django.urls import reverse_lazy
from django.views.generic import CreateView

class SignUpView(CreateView):
    form_class = UserCreationForm
    success_url = reverse_lazy('login') # Redirige vers la page de connexion après une inscription réussie
    template_name = 'registration/signup.html'