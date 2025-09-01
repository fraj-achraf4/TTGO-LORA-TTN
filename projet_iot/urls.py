from django.contrib import admin
from django.urls import path, include
from django.contrib.auth.views import LogoutView # <-- LA LIGNE IMPORTANTE EST ICI

urlpatterns = [
    path('admin/', admin.site.urls),
    
    # Votre règle personnalisée pour le logout
    path('accounts/logout/', LogoutView.as_view(next_page='home'), name='logout'),

    # Les autres règles pour l'authentification
    path('accounts/', include('django.contrib.auth.urls')),
    path('accounts/', include('users.urls')),
    
    path('', include('dashboard.urls')),
]