from django.contrib import admin
from django.urls import path, include


#urls for root apps site
urlpatterns = [
    path('admin/', admin.site.urls),
    path ('', include('base.urls')),   
]
