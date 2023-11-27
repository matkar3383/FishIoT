from django.urls import path
from . import views   
from django.http import HttpResponse
 
  
# urls for specific app

urlpatterns = [
    path('', views.home, name="home"),
    path('room/<str:pk>/', views.room, name="room"),
]

