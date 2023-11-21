from django.urls import path
from . import views 

urlpatterns = [
    path('', views.home, name="home"),
    path('send_check_request/', views.send_check_request, name="send_check_request"),
    path('send_set_request/', views.send_set_request, name="send_set_request"),
    path('disconnect/', views.disconnect, name="disconnect")
]