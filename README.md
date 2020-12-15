# Esports-Scoreboard

In this report I will be looking at the esports project brief and evaluating the requests and demands of the project, the brief demands that we have got a scoreboard that displays the scores of everyone involved within the game in a nice simple easy to read layout, and allow judges to enter the results of each game as the finish them, we are required to support at least 6 teams each with 5 members who will be able to compete in 5 different games, when these games are finished they get given a placement rank witch will then dictate the amount of points they get given, it was specifically stated that the points given for each rank are still undecided so we can decide that on our own, they also stated in the brief that there needs to be a way for a guest team entering for one game only. Later, we then spoke to the client with some questions.

So we questioned the client on different graphic design questions such as the colour and art style they wanted and told us that they have no specific art styles they are going for, but in regards to the colour palate they said they would like them based of the colleges colour palate, so that’s something that I will need to include in the final design to cater to their requirements, 
I also asked them about the deployment of the final build and we were told it would be displayed on a large screen for a crowd or group of people on a stream or LAN tournament screen, and also on a judges computer for them to enter the scores manually, they also specified that they would like support for there to be multiple people entering scores simultaneously, so again this is something we need to take into consideration when working on the final build, I then went on to ask about the platforms of these clients and was told they would all be windows based so there isn’t need for cross platform support.




### First scoring system design

My first design for the scoring system is setup so that you would play a game between 2 or more teams then the placements of each of the teams will be entered into the system and then the system will allocate points to each team based on their placement were the higher placements will result in the team gaining more points ranking them higher than others using the scores of 
1st being worth 4 points, 
2nd being worth 3 points, 
3rd being worth 2 points, 
and anything else being worth 1 point
and if someone draws 1st 2nd or 3rd it will count them both as coming the rank below and the rank, they both came is skipped.
I believe this is a good idea because it will mean that people who win higher ranks more often will rank higher overall and also the gap between each position is marginal it gives others more chance to be able to overtake each other, but this means that the games direction can change very quickly one minute someone can be in first then next game be in last, there’s a lot of pressure on the person holding 1st place to keep it, also in regards to skipping I believe stopping them both from getting that extra points as they drew will make situations a lot more stressful and frustrating for the players making the game more interesting, as it gives less incentive to want to draw.


### Second Scoreing system design

My second idea for a scoring system is having a system similar to the first meaning after each game everyone will get a placement 1st, 2nd and 3rd etc, and that will dictate the number of points they get but the gap between each of the placements points they get is calculated with an exponential equation like
I = placement number
(I / 3) * 2 = Points
I would believe this is a great method pf scoring  because then it makes it a lot easier for people to gain the lead again after losing it as the boundaries are a lot closer than with the other systems, I have come up with.


For my initial design for the layout, I have decided to create this

## Image 1

Within this design I have chosen to have multiple areas of the application first the actual leader board that displays all the current positions and standings of all the players and teams, this shows the points per team and gives for the ability to view the points each team member has gained for their team, also on this screen it will display the overall wins/losses for each of these teams, as well as the team stats it will show the stats for each individual game.

To change/add new team there are confusing menus for the admins to navigate and modify all the game settings, features and competing players I don’t believe the layout of these menus are simple to navigate and change the settings for.



### GUI Design one

This is my main gui  design

## Image 2

With this design I believe I have cleared up a lot of the confusion with the design and general usage of the application, with this design everything is in one window, but there are 2 operating modes of this application, spectator mode and admin mode within spectator mode all the add new/change buttons/input fields are unusable and hidden from the operator but in admin mode all of them fields are usable and visible.


### Development issues

While starting the development of this software I had raised a few concerns in regards to saving the data  I first attempted to use a JSON file for managing everything in regards to storing everything and keeping everything persistent, but I found this approach to be very time consuming and inefficient. I instead opted to instead convert the mainStorage var that stores everything into a BYTE array and then write them raw BYTEs to a file, then on load of the application it will load them RAW bytes and convert them into the correct objects. Completing the Save/Load cycle.

During development ive decided that I will have to add in the saving/sterilisation later on in the project so Ive pushed that further down the backlog as currently it will require a lot of work to create a fast and efficient custom implementation.
While creating the final design and production of the scoreboard I decided functionality was less important than the GUI as I can rework the GUI in a later releace 
