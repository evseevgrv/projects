from flask import Flask, render_template, session, redirect, request, url_for, jsonify, session, g
from flask_sqlalchemy import SQLAlchemy
from flask_mail import Mail, Message
from itsdangerous import URLSafeTimedSerializer
import bcrypt
import string
import random

application = Flask(__name__)

application.config['SECRET_KEY'] = 'xR"V\c5%*c^raLV{j@5BD??".\&Fe+'
application.config['DEBUG'] = True
application.config['TESTING'] = False

application.config['MAIL_SERVER'] = 'smtp.gmail.com'  # настройки почты
application.config['MAIL_PORT'] = 587
application.config['MAIL_USE_TLS'] = True
application.config['MAIL_USE_SSL'] = False
application.config['MAIL_USERNAME'] = 'pochta.helpivr@gmail.com'
application.config['MAIL_PASSWORD'] = 'hewvnsktixevygno'
application.config['MAIL_DEFAULT_SENDER'] = 'pochta.helpivr@gmail.com'
application.config['MAIL_MAX_EMAILS'] = None
application.config['MAIL_ASCII_ATTACHMENTS'] = False
mail = Mail(application)

s = URLSafeTimedSerializer(application.config['SECRET_KEY'])
# расположение базы данных
application.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///main.db'

application.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(application)


def gen_pw(chars=string.ascii_uppercase + string.digits):  # генерация нового пароля
    return ''.join(random.choice(chars) for _ in range(16))


class Posts(db.Model):  # таблица данных постов
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer)
    user_name = db.Column(db.String)
    user_surname = db.Column(db.String)
    # предложение - 1, резюме - 2, опрос - 3, идея - 4
    post_type = db.Column(db.Integer)
    project_type = db.Column(db.String)  # тип ИВР
    subject = db.Column(db.String)  # предмет ИВР
    problem_type = db.Column(db.String)  # сфера поставленной задачи
    name = db.Column(db.String)
    demands = db.Column(db.String)
    description = db.Column(db.String)
    href_vk = db.Column(db.String)
    href_telegram = db.Column(db.String)
    href_google = db.Column(db.String)
    href_quiz = db.Column(db.String)
    date = db.Column(db.String)

    def __repr__(self):
        return f"<posts {self.id}>"

    def __init__(self, user_id, user_name, user_surname,
                 post_type, project_type, subject, problem_type, name, demands, description,
                 href_vk, href_telegram, href_google, href_quiz, date):
        self.user_id = user_id
        self.user_name = user_name
        self.user_surname = user_surname
        self.post_type = post_type
        self.project_type = project_type
        self.subject = subject
        self.problem_type = problem_type
        self.name = name
        self.demands = demands
        self.description = description
        self.href_vk = href_vk
        self.href_telegram = href_telegram
        self.href_google = href_google
        self.href_quiz = href_quiz
        self.date = date
    #


class Users(db.Model):  # таблица данных пользователей
    id = db.Column(db.Integer, primary_key=True)
    user_nickname = db.Column(db.String, unique=True)
    user_email = db.Column(db.String, unique=True)
    user_name = db.Column(db.String)
    user_surname = db.Column(db.String)
    user_group = db.Column(db.String)
    user_password = db.Column(db.String)
    check_email = db.Column(db.Integer)
    href_vk = db.Column(db.String)
    href_telegram = db.Column(db.String)

    def __repr__(self):
        return f"<users {self.id}>"

    def __init__(self, user_nickname, user_email, user_name, user_surname, user_group, user_password):
        self.user_nickname = user_nickname
        self.user_email = user_email
        self.user_name = user_name
        self.user_surname = user_surname
        self.user_group = user_group
        self.user_password = user_password
        self.check_email = 0
        self.href_vk = '-'
        self.href_telegram = '-'


@application.before_request
def before_request():
    g.name = None
    g.surname = None
    g.usid = None
    g.check_email = None
    g.email = None
    g.href_vk = None
    g.href_telegram = None

    if 'name' in session and 'surname' in session and 'usid' in session and 'check_email' in session and 'email' in session and 'href_vk' in session and 'href_telegram' in session:
        g.name = session['name']
        g.surname = session['surname']
        g.usid = session['usid']
        g.check_email = session['check_email']
        g.email = session['email']
        g.href_vk = session['href_vk']
        g.href_telegram = session['href_telegram']


@application.route('/dropsession')  # выход из аккаунта
def dropsession():
    session.pop('name', None)
    session.pop('surname', None)
    session.pop('usid', None)
    session.pop('message', None)
    session.pop('check_email', None)
    session.pop('email', None)
    session.pop('href_vk', None)
    session.pop('href_telegram', None)
    return redirect(url_for('index'))


@application.route('/confirm_email/<token>')  # ссылка для подтверждения почты
def confirm_email(token):
    email = s.loads(token, salt='email-confirm')
    us_confirm = db.session.query(Users).filter(
        Users.user_email == email).first()  # получаем пользователя по его почте
    us_confirm.check_email = 1  # подтверждаем, что почта проверена
    db.session.commit()
    session['check_email'] = 1
    return render_template('confirm_email.html')


# для повторной отправки письма
@application.route('/get_confirm/<int:id>', methods=['GET', 'POST'])
def get_confirm(id):
    us_confirm = db.session.query(Users).filter(Users.id == id).first()
    if request.method == 'POST':
        email = request.form["email"]
        token = s.dumps(email, salt='email-confirm')
        msg = Message('Подтвердите почту', recipients=[email])
        link = url_for('confirm_email', token=token, _external=True)
        if email != us_confirm.user_email:  # если пользователь ввел новую почту, то она изменяется
            oluser = db.session.query(Users).filter(
                Users.user_email == email).first()  # проверяем, занята ли почта
            if oluser:
                session['message'] = 'Почта уже занята'
                return redirect(url_for('get_confirm', id=id))
            us_confirm.user_email = email
            db.session.commit()
            msg.body = 'Ваша почта была изменена. Для подтверждения почты, пожалуйста, перейдите по ссылке {}'.format(
                link)
        else:
            msg.body = 'Для подтверждения почты, пожалуйста, перейдите по ссылке {}'.format(
                link)
        mail.send(msg)
        return redirect(url_for('index'))
    mes = session.get('message', '')
    if mes != 'Почта уже занята':
        mes = ''
        session['message'] = ''
    return render_template('get_confirm.html', user=us_confirm, message=mes)


# получить свои данные для входа
@application.route("/get_info", methods=['GET', 'POST'])
def get_info():
    if request.method == 'POST':
        email = request.form["email"]
        user = db.session.query(Users).filter(
            Users.user_email == email).first()  # находим пользователя по почте
        if user is not None:  # проверяем, что пользователь с такой почтой существует
            msg = Message('Данные для входа', recipients=[email])
            password = gen_pw()  # генерируем новый пароль
            hashed = bcrypt.hashpw(password.encode(
                'utf-8'), bcrypt.gensalt())  # кодируем пароль
            user.user_password = hashed.decode('utf-8')
            db.session.commit()
            msg.body = 'Данные для входа: логин - {}, новый пароль - {}. Установить новый логин или пароль вы можете в разделе Мои данные.'.format(
                user.user_nickname, password)
            mail.send(msg)  # отправляем письмо с данными
            return redirect(url_for('index'))
        else:
            session['message'] = 'Пользователя с такой почтой не существует'
            return redirect(url_for('get_info'))
    mes = session.get('message', '')
    if mes != 'Пользователя с такой почтой не существует':
        mes = ''
        session['message'] = ''
    return render_template('get_info.html', message=mes)


@application.route("/", methods=['GET', 'POST'])
def index():
    if g.name and g.surname and g.usid:  # проверка, что пользователь уже вошел в свой аккаунт и не выходил из него
        all_posts = db.session.query(Posts).all()
        post = []
        vacancy = []
        quiz = []
        idea = []
        for i in all_posts:  # создаем списпки постов
            if i.post_type == 1:
                if i.user_id != g.usid:  # добавляем посты только других пользователей
                    post.append([
                        i.user_name, i.user_surname, i.project_type,
                        i.subject, i.problem_type, i.name, i.demands, i.description,
                        i.href_vk, i.href_telegram, i.href_google
                    ])
            elif i.post_type == 2:
                if i.user_id != g.usid:
                    vacancy.append([
                        i.user_name, i.user_surname,
                        i.problem_type, i.description,
                        i.href_vk, i.href_telegram, i.href_google
                    ])
            elif i.post_type == 3:
                if i.user_id != g.usid:
                    quiz.append([
                        i.user_name, i.user_surname,
                        i.name, i.description,
                        i.href_vk, i.href_telegram, i.href_google, i.href_quiz
                    ])
            elif i.post_type == 4:
                if i.user_id != g.usid:
                    idea.append([
                        i.project_type, i.subject, i.name, i.description
                    ])

            post.reverse()
            vacancy.reverse()
            quiz.reverse()
            idea.reverse()

        session['message'] = ''
        return render_template('index.html', name=session['name'], surname=session['surname'], usid=session['usid'],
                               check_email=session['check_email'],
                               post=post, vacancy=vacancy, quiz=quiz, idea=idea)

    if request.method == 'POST':
        session.pop('name', None)
        session.pop('surname', None)
        session.pop('usid', None)

        if request.form["btn"] == "Войти":  # если происходит операция входа
            nickname = request.form["nickname"]  # получаем данные
            password = request.form["password"]

            user = db.session.query(Users).filter(
                Users.user_nickname == nickname).first()  # находим пользователя по логину
            if user:  # проверяем существование пользователя с данным логином
                # проверяем корректность пароля
                if bcrypt.checkpw(password.encode('utf-8'), user.user_password.encode('utf-8')):
                    name = user.user_name
                    surname = user.user_surname
                else:
                    session['message'] = 'Неправильный логин или пароль'
                    return redirect(url_for('index'))
            else:
                session['message'] = 'Неправильный логин или пароль'
                return redirect(url_for('index'))

            session['name'] = name
            session['surname'] = surname
            session['usid'] = user.id
            session['message'] = ''
            session['check_email'] = user.check_email
            session['email'] = user.user_email
            session['href_vk'] = user.href_vk
            session['href_telegram'] = user.href_telegram

        # если происходит операция регистрации
        elif request.form["btn"] == "Зарегистрироваться":
            nickname = request.form["nickname"]
            # проверяем наличие пользователей с введенным логином
            oluser = db.session.query(Users).filter(
                Users.user_nickname == nickname).first()
            if oluser:
                session['message'] = 'Логин уже занят'
                return redirect(url_for('index'))

            email = request.form["email"]
            # проверяем наличие пользователей с введенной почтой
            oluser = db.session.query(Users).filter(
                Users.user_email == email).first()
            if oluser:
                session['message'] = 'Почта уже занята'
                return redirect(url_for('index'))
            name = request.form["name"]
            surname = request.form["surname"]
            group = request.form["group"]
            password = request.form["password"]
            hashed = bcrypt.hashpw(password.encode(
                'utf-8'), bcrypt.gensalt())  # кодируем пароль

            user = Users(nickname, email, name, surname, group,
                         hashed.decode('utf-8'))  # создаем нового пользователя
            db.session.add(user)
            db.session.commit()
            # отправляем письмо для подтверждения почты
            token = s.dumps(email, salt='email-confirm')
            msg = Message('Подтвердите почту', recipients=[email])
            link = url_for('confirm_email', token=token, _external=True)
            msg.body = 'Для подтверждения почты, пожалуйста, перейдите по ссылке {}'.format(
                link)
            mail.send(msg)

            session['name'] = name
            session['surname'] = surname
            session['usid'] = user.id
            session['message'] = ''
            session['check_email'] = user.check_email
            session['email'] = user.user_email
            session['href_vk'] = user.href_vk
            session['href_telegram'] = user.href_telegram

        all_posts = db.session.query(Posts).all()
        post = []
        vacancy = []
        quiz = []
        idea = []
        for i in all_posts:  # добавлям посты
            if i.post_type == 1:
                if i.user_id != user.id:  # добавляем посты только других пользователей
                    post.append([
                        i.user_name, i.user_surname, i.project_type,
                        i.subject, i.problem_type, i.name, i.demands, i.description,
                        i.href_vk, i.href_telegram, i.href_google
                    ])
            elif i.post_type == 2:
                if i.user_id != user.id:
                    vacancy.append([
                        i.user_name, i.user_surname,
                        i.problem_type, i.description,
                        i.href_vk, i.href_telegram, i.href_google
                    ])
            elif i.post_type == 3:
                if i.user_id != user.id:
                    quiz.append([
                        i.user_name, i.user_surname,
                        i.name, i.description,
                        i.href_vk, i.href_telegram, i.href_google, i.href_quiz
                    ])
            elif i.post_type == 4:
                if i.user_id != user.id:
                    idea.append([
                        i.user_name, i.user_surname, i.project_type,
                        i.subject, i.problem_type, i.name, i.description,
                        i.href_vk, i.href_telegram, i.href_google
                    ])

            post.reverse()
            vacancy.reverse()
            quiz.reverse()
            idea.reverse()

        return render_template('index.html', surname=surname, name=name, usid=session['usid'],
                               check_email=session['check_email'],
                               post=post, vacancy=vacancy, quiz=quiz, idea=idea)
    mes = session.get('message', '')
    if mes != 'Неправильный логин или пароль' and mes != 'Логин уже занят' and mes != 'Почта уже занята':
        session['message'] = ''
        mes = ''
    return render_template('login.html', message=mes)


# просмотр и изменение данных пользователя
@application.route('/edit/<int:id>', methods=['GET', 'POST'])
def edit(id):
    usid = session.get('usid', -1)
    if id != usid:
        return redirect(url_for('index'))
    us_update = db.session.query(Users).filter(
        Users.id == id).first()  # находим пользователя по id
    if request.method == 'POST':
        nickname = request.form["nickname"]
        if nickname != us_update.user_nickname:  # если логин был изменен
            oluser = db.session.query(Users).filter(
                Users.user_nickname == nickname).first()  # проверяем, занят ли логин
            if oluser:
                session['message'] = 'Логин уже занят'
                return redirect(url_for('edit', id=id))
            us_update.user_nickname = nickname

        email = request.form["email"]
        if email != us_update.user_email:  # если почта была изменена
            oluser = db.session.query(Users).filter(
                Users.user_email == email).first()  # проверяем, занята ли почта
            if oluser:
                session['message'] = 'Почта уже занята'
                return redirect(url_for('edit', id=id))
            us_update.user_email = email  # меняем почту
            us_update.check_email = 0
            # отправляем письмо для подтверждения новой почты
            token = s.dumps(email, salt='email-confirm')
            msg = Message('Подтвердите почту', recipients=[email])
            link = url_for('confirm_email', token=token, _external=True)
            msg.body = 'Для подтверждения почты, пожалуйста, перейдите по ссылке {}'.format(
                link)
            mail.send(msg)

        us_update.user_name = request.form["name"]
        us_update.user_surname = request.form["surname"]
        us_update.user_group = request.form["group"]
        if request.form["href_vk"] != "":  # проверяем, был ли введен id аккаунта ВКонтакте
            us_update.href_vk = request.form["href_vk"]
        else:  # если нет, задаем значение по умолчанию
            us_update.href_vk = '-'
        if request.form["href_telegram"] != "":
            us_update.href_telegram = request.form["href_telegram"]
        else:
            us_update.href_telegram = '-'
        password = request.form["password"]
        if len(password) != 0:
            hashed = bcrypt.hashpw(password.encode('utf-8'), bcrypt.gensalt())
            us_update.user_password = hashed.decode('utf-8')
        session['name'] = us_update.user_name
        session['surname'] = us_update.user_surname
        session['usid'] = us_update.id
        session['message'] = ''
        session['check_email'] = us_update.check_email
        session['email'] = us_update.user_email
        session['href_vk'] = us_update.href_vk
        session['href_telegram'] = us_update.href_telegram
        db.session.commit()
        return redirect(url_for('index'))
    mes = session.get('message', '')
    return render_template('edit.html', user=us_update, message=mes)


@application.route('/myposts', methods=['GET'])  # просмотр своих постов
def myposts():
    all_posts = db.session.query(Posts).filter(  # ищем все посты пользователя по его id
        Posts.user_id == g.usid
    ).all()
    post = []
    vacancy = []
    quiz = []
    idea = []
    for i in all_posts:
        if i.post_type == 1:
            post.append([
                i.user_name, i.user_surname, i.project_type,
                i.subject, i.problem_type, i.name, i.demands, i.description,
                i.href_vk, i.href_telegram, i.href_google, i.id
            ])
        elif i.post_type == 2:
            vacancy.append([
                i.user_name, i.user_surname,
                i.problem_type, i.description,
                i.href_vk, i.href_telegram, i.href_google, i.id
            ])
        elif i.post_type == 3:
            quiz.append([
                i.user_name, i.user_surname,
                i.name, i.description,
                i.href_vk, i.href_telegram, i.href_google, i.href_quiz, i.id
            ])
        elif i.post_type == 4:
            idea.append([
                i.user_name, i.user_surname, i.project_type,
                i.subject, i.problem_type, i.name, i.description,
                i.href_vk, i.href_telegram, i.href_google, i.id
            ])

    return render_template('myposts.html', post=post, vacancy=vacancy, quiz=quiz, idea=idea)


@application.route('/delete/<int:id>')  # удаление поста
def delete(id):
    usid = session.get('usid', -1)
    post_delete = db.session.query(Posts).filter(Posts.id == id).first()
    if post_delete.user_id != usid:
        return redirect(url_for('index'))
    db.session.delete(post_delete)
    db.session.commit()

    return redirect(url_for('myposts'))


# измененние предложения
@application.route('/update/post/<int:id>', methods=['GET', 'POST'])
def update_post(id):
    usid = session.get('usid', -1)
    post_update = db.session.query(Posts).filter(Posts.id == id).first()
    if post_update.user_id != usid:
        return redirect(url_for('index'))
    if request.method == 'POST':
        # post_update = request.form['name']

        post_update.project_type = request.form.get('type')
        post_update.subject = request.form.get('subject')
        post_update.problem_type = request.form.get('problemtype')
        post_update.name = request.form["name"]
        post_update.demands = request.form["demands"]
        post_update.description = request.form["description"]
        if request.form["href_vk"] != "":
            post_update.href_vk = request.form["href_vk"]
        else:
            post_update.href_vk = '-'
        if request.form["href_telegram"] != "":
            post_update.href_telegram = request.form["href_telegram"]
        else:
            post_update.href_telegram = '-'
        post_update.href_google = request.form["href_google"]
        db.session.commit()

        return redirect(url_for('myposts'))
    return render_template('update_post.html', post_update=post_update)


# изменение резюме
@application.route('/update/vacancy/<int:id>', methods=['GET', 'POST'])
def update_vacancy(id):
    usid = session.get('usid', -1)
    post_update = db.session.query(Posts).filter(Posts.id == id).first()
    if post_update.user_id != usid:
        return redirect(url_for('index'))
    if request.method == 'POST':
        # post_update = request.form['name']

        post_update.problem_type = request.form.get('problemtype')
        post_update.description = request.form['description']
        if request.form["href_vk"] != "":
            post_update.href_vk = request.form["href_vk"]
        else:
            post_update.href_vk = '-'
        if request.form["href_telegram"] != "":
            post_update.href_telegram = request.form["href_telegram"]
        else:
            post_update.href_telegram = '-'
        post_update.href_google = request.form['href_google']
        db.session.commit()

        return redirect(url_for('myposts'))
    return render_template('update_vacancy.html', post_update=post_update)


# измененние поста для опроса
@application.route('/update/quiz/<int:id>', methods=['GET', 'POST'])
def update_quiz(id):
    usid = session.get('usid', -1)
    post_update = db.session.query(Posts).filter(Posts.id == id).first()
    if post_update.user_id != usid:
        return redirect(url_for('index'))
    if request.method == 'POST':
        # post_update = request.form['name']

        post_update.name = request.form['name']
        post_update.description = request.form['description']
        post_update.href_quiz = request.form['href_quiz']
        if request.form["href_vk"] != "":
            post_update.href_vk = request.form["href_vk"]
        else:
            post_update.href_vk = '-'
        if request.form["href_telegram"] != "":
            post_update.href_telegram = request.form["href_telegram"]
        else:
            post_update.href_telegram = '-'
        post_update.href_google = request.form['href_google']
        db.session.commit()

        return redirect(url_for('myposts'))
    return render_template('update_quiz.html', post_update=post_update)


# изменение поста с идеей
@application.route('/update/idea/<int:id>', methods=['GET', 'POST'])
def update_idea(id):
    usid = session.get('usid', -1)
    post_update = db.session.query(Posts).filter(Posts.id == id).first()
    if post_update.user_id != usid:
        return redirect(url_for('index'))
    if request.method == 'POST':
        # post_update = request.form['name']

        post_update.project_type = request.form.get('type')
        post_update.subject = request.form.get('subject')
        post_update.problem_type = request.form.get('problemtype')
        post_update.name = request.form['name']
        post_update.description = request.form['description']

        db.session.commit()

        return redirect(url_for('myposts'))
    return render_template('update_idea.html', post_update=post_update)


# добавление предложения
@application.route('/add_post', methods=['GET', 'POST'])
def add_post():
    if request.method == 'POST':
        user_id = session['usid']
        user_name = session['name']
        user_surname = session['surname']
        post_type = 1
        project_type = request.form.get('type')
        subject = request.form.get('subject')
        problem_type = request.form.get('problemtype')
        name = request.form["name"]
        demands = request.form["demands"]
        description = request.form["description"]
        if request.form["href_vk"] != "":
            # если пользователь ввел id аккаунта ВКонтаке
            href_vk = request.form["href_vk"]
        else:
            href_vk = '-'  # в противном случае задается значение по умолчанию
        if request.form["href_telegram"] != "":
            href_telegram = request.form["href_telegram"]
        else:
            href_telegram = '-'
        href_google = request.form["href_google"]
        href_quiz = ''
        date = ''

        post = Posts(user_id, user_name, user_surname, post_type,
                     project_type, subject, problem_type, name, demands, description,
                     href_vk, href_telegram, href_google, href_quiz, date)
        db.session.add(post)
        db.session.commit()

        return redirect(url_for('index'))
    # если у пользователя подтверждена почта, то она вводится автоматически
    if session['check_email'] == 1:
        email = session['email']
    else:
        email = ''
    # если пользователь добавил id аккаунта ВКонтакте и Telegram, то данные введутся автоматически
    return render_template('add_post.html', email=email, href_vk=session['href_vk'],
                           href_telegram=session['href_telegram'])


# добавление резюме
@application.route('/add_vacancy', methods=['GET', 'POST'])
def add_vacancy():
    if request.method == 'POST':
        user_id = session['usid']
        user_name = session['name']
        user_surname = session['surname']
        post_type = 2
        problem_type = request.form.get('problemtype')
        description = request.form['description']
        if request.form["href_vk"] != "":
            href_vk = request.form["href_vk"]
        else:
            href_vk = '-'
        if request.form["href_telegram"] != "":
            href_telegram = request.form["href_telegram"]
        else:
            href_telegram = '-'
        href_google = request.form['href_google']

        project_type = ''
        subject = ''
        name = ''
        demands = ''
        href_quiz = ''
        date = ''

        post = Posts(user_id, user_name, user_surname, post_type,
                     project_type, subject, problem_type, name, demands, description,
                     href_vk, href_telegram, href_google, href_quiz, date)
        db.session.add(post)
        db.session.commit()

        return redirect(url_for('index'))

    if session['check_email'] == 1:
        email = session['email']
    else:
        email = ''
    return render_template('add_vacancy.html', email=email, href_vk=session['href_vk'],
                           href_telegram=session['href_telegram'])


# добавление поста с опросом
@application.route('/add_quiz', methods=['GET', 'POST'])
def add_quiz():
    if request.method == 'POST':
        user_id = session['usid']
        user_name = session['name']
        user_surname = session['surname']
        post_type = 3
        name = request.form['name']
        description = request.form['description']
        href_quiz = request.form['href_quiz']
        if request.form["href_vk"] != "":
            href_vk = request.form["href_vk"]
        else:
            href_vk = '-'
        if request.form["href_telegram"] != "":
            href_telegram = request.form["href_telegram"]
        else:
            href_telegram = '-'
        href_google = request.form['href_google']

        project_type = ''
        subject = ''
        problem_type = ''
        demands = ''
        date = ''

        post = Posts(user_id, user_name, user_surname, post_type,
                     project_type, subject, problem_type, name, demands, description,
                     href_vk, href_telegram, href_google, href_quiz, date)
        db.session.add(post)
        db.session.commit()

        return redirect(url_for('index'))

    if session['check_email'] == 1:
        email = session['email']
    else:
        email = ''
    return render_template('add_quiz.html', email=email, href_vk=session['href_vk'],
                           href_telegram=session['href_telegram'])


# добавление поста с идеей
@application.route('/add_idea', methods=['GET', 'POST'])
def add_idea():
    if request.method == 'POST':
        user_id = session['usid']
        user_name = session['name']
        user_surname = session['surname']
        post_type = 4
        project_type = ''
        subject = request.form['subject']
        problem_type = ''
        name = request.form['name']
        description = request.form['description']

        demands = ''
        href_vk = ''
        href_telegram = ''
        href_google = ''
        href_quiz = ''
        date = ''

        post = Posts(user_id, user_name, user_surname, post_type,
                     project_type, subject, problem_type, name, demands, description,
                     href_vk, href_telegram, href_google, href_quiz, date)
        db.session.add(post)
        db.session.commit()

        return redirect(url_for('index'))

    return render_template('add_idea.html')


if __name__ == "__main__":  # начало работы
    application.run(host='0.0.0.0')
