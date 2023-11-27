from django.shortcuts import render

#function calling the page
rooms = [
    {'id': 1, 'name': 'zagrajmy'},
    {'id': 2, 'name': 'przegrajmy'},
    {'id': 3, 'name': 'chuj wie -- test'},
]

def home(request):
    context = {'rooms':rooms}
    return render(request, 'base/home.html', context)

def room(request, pk):
    room = None
    for i in rooms:
        if i['id'] == int(pk):
            room = i
    context = {'room', room}
    return render(request, 'base/room.html', context)

##TODO tu jest błąd gdzieś!!!